#include "object_detection.hpp"

std::tuple<torch::Tensor, dnn::tensor_scale> dnn::object_detection::preprocess( cv::Mat img )
{
    cv::cvtColor( img, img, cv::COLOR_BGR2RGB );
    cv::Size original_size = img.size();

    double scale = dnn::tools::calculate_scale( original_size, max_side );

    cv::Mat img_scaled;
    cv::resize( img, img_scaled, cv::Size(), scale, scale );
    cv::Size new_size = img_scaled.size();

    torch::Tensor out = torch::zeros({ 3, max_side, max_side });
    out = out.to( torch::kFloat32 );

    //toTensor
    torch::Tensor tensor_image = torch::from_blob(img_scaled.data, {img_scaled.rows, img_scaled.cols, 3}, at::kByte);

    // Mapping the values to 0 and 1
    tensor_image = tensor_image.to( torch::kFloat32 ) / 255.0;

    tensor_image = tensor_image.permute({2,0,1});

    // Normalizing the tensor
    tensor_image[0] = (tensor_image[0] - mean[0]) / std[0];
    tensor_image[1] = (tensor_image[1] - mean[1]) / std[1];
    tensor_image[2] = (tensor_image[2] - mean[2]) / std[2];

    out.index({torch::indexing::Slice(), 
               torch::indexing::Slice(0,new_size.height), 
               torch::indexing::Slice(0,new_size.width)}) = tensor_image;

    out = out.unsqueeze_(0);

    dnn::tensor_scale tscale;
    tscale.original_size = original_size;
    tscale.size = new_size;
    tscale.hscale = static_cast<float>( new_size.height ) / static_cast<float>( original_size.height );
    tscale.wscale = static_cast<float>( new_size.width ) / static_cast<float>( original_size.width );


    return { out, tscale };
}

std::vector<dnn::detections> dnn::object_detection::postprocess( std::vector<torch::Tensor>& cls_probs, 
                                                                 std::vector<torch::Tensor>& bbox_regression,
                                                                 dnn::tensor_scale scale )
{
    auto meshes = dnn::tools::build_meshgrid( bbox_regression, max_side );
    auto rois = dnn::tools::map_bbox_regression( bbox_regression, meshes );

    auto cls_probs_tensor = dnn::tools::permute_flatten_cat( cls_probs ).contiguous();
    auto rois_tensor = dnn::tools::permute_flatten_cat( rois ).contiguous();

    int num_classes = cls_probs_tensor.sizes()[1];

    std::vector<dnn::detections> detections;

    for( int cls_idx=0 ; cls_idx<num_classes ; cls_idx++ )
    {
        auto scores = cls_probs_tensor.index({torch::indexing::Slice(), cls_idx});

        torch::Tensor cls_rois;

        if( unified_rois )
        {
            cls_rois = rois_tensor;
        }
        else
        {
            auto indices = torch::arange(0,4) + 4 * cls_idx;
            cls_rois = rois_tensor.index({torch::indexing::Slice(), indices});
        }

        auto keep = scores >= score_thresh;

        scores = scores.index({keep});
        cls_rois = cls_rois.index({keep});
        cls_rois = dnn::tools::scale_rois( cls_rois, scale.hscale, scale.wscale );        
        cls_rois = dnn::tools::clip_rois( cls_rois, scale.original_size );

        auto nms_keep = vision::ops::nms( cls_rois, scores, nms_thresh );
        scores = scores.index({nms_keep});
        cls_rois = cls_rois.index({nms_keep});

        detections.push_back( {cls_idx, class_names[cls_idx], scores, cls_rois} );
    }

    return detections;
}

dnn::object_detection::object_detection(  YAML::Node cfg )
{
    max_side = cfg["max_side"].as<int>();
    std::string model_path = cfg["model_path"].as<std::string>();
    score_thresh = cfg["score_thresh"].as<float>();
    nms_thresh = cfg["nms_thresh"].as<float>();
    unified_rois = cfg["unified_rois"].as<bool>();

    mean = cfg["image_mean"].as<std::vector<float>>();
    std = cfg["image_std"].as<std::vector<float>>();

    class_names = cfg["class_names"].as<std::map<int,std::string>>();

    try 
    {
        torch::NoGradGuard no_grad;
        // Deserialize the ScriptModule from a file using torch::jit::load().
        module = torch::jit::load(model_path);
    }
    catch (const c10::Error& e) 
    {
        std::cerr << "error loading the model\n";
        exit(-1);
    }
    catch (const std::exception& e) 
    {
        std::cout << "Other error: " << e.what() << "\n";
        exit(-1);
    }

    module.eval();
}

std::vector<dnn::detections> dnn::object_detection::process( cv::Mat img )
{
    auto [ tensor, scale ] = preprocess( img );

    //std::cout << tensor.sizes() << std::endl;

    //auto t0 = std::chrono::steady_clock::now();
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back( tensor );
    //std::cout << module.requires_grad() << std::endl;
    auto out = module.forward( inputs ).toTuple();
    //auto t1 = std::chrono::steady_clock::now();

    //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>( t1 - t0 ).count() << std::endl;

    std::vector<torch::Tensor> cls_probs;
    std::vector<torch::Tensor> bbox_regression;

    int count = out->elements().size();

    for( int i=0 ; i<count/2 ; i++ )
    {
        auto cls_t = torch::sigmoid(out->elements()[i].toTensor()).contiguous().cpu();
        auto bbox_t = out->elements()[i + count/2].toTensor().contiguous().cpu(); 

        cls_probs.push_back( cls_t );
        bbox_regression.push_back( bbox_t );
    }

    return postprocess( cls_probs, bbox_regression, scale );
}
