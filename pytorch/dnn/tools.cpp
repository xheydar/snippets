#include "tools.hpp"

cv::Vec3f dnn::tools::normalize_pixel( cv::Vec3f pixel, cv::Vec3f mean, cv::Vec3f std )
{
    pixel[0] = (pixel[0] - mean[0])/std[0];
    pixel[1] = (pixel[1] - mean[1])/std[1];
    pixel[2] = (pixel[2] - mean[2])/std[2];

    return pixel;
}

void dnn::tools::normalize_image( cv::Mat img_float, cv::Vec3d mean, cv::Vec3f std )
{
    for( int i=0 ; i<img_float.rows ; i++ )
    {
        for( int j=0 ; j<img_float.cols ; j++ )
        {
            img_float.at<cv::Vec3f>(i,j) = dnn::tools::normalize_pixel( img_float.at<cv::Vec3f>(i,j), mean, std );
        }
    }
}

double dnn::tools::calculate_scale( cv::Size s, int max_side )
{
    int largest_side = std::max( s.height, s.width );
    return static_cast<double>( max_side ) / largest_side;
}

std::vector<dnn::tools::mesh> dnn::tools::build_meshgrid( std::vector<torch::Tensor> &fmaps, int image_size )
{
    std::vector<dnn::tools::mesh> meshes;

    for( auto &fmap : fmaps )
    {
        auto shape = fmap.sizes();

        int grid_height = shape[2];
        int grid_width = shape[3];

        int stride_height = image_size / grid_height;
        int stride_width = image_size / grid_width;

        auto x_linspace = (torch::linspace(0, grid_width-1, grid_width).to(torch::kFloat32) + 0.5) * stride_width;
        auto y_linspace = (torch::linspace(0, grid_height-1, grid_height).to(torch::kFloat32) + 0.5 ) * stride_height;

        auto args = torch::meshgrid( {y_linspace, x_linspace}, "ij" );
        auto yy = args[0];
        auto xx = args[1];
        auto scale = log2( static_cast<float>(stride_width) );
        
        meshes.push_back( { xx,yy, x_linspace, y_linspace, scale } );
    }

    return meshes;
}

std::vector<torch::Tensor> dnn::tools::map_bbox_regression( std::vector<torch::Tensor>& rois_net, std::vector<dnn::tools::mesh>& meshes )
{
    std::vector<torch::Tensor> out;

    for( int i=0 ; i<rois_net.size() ; i++ )
    {
        auto tensor_rois = rois_net[i];
        double scale = pow(2, meshes[i].scale );
        auto tensor_rois_size = tensor_rois.sizes();

        int height = tensor_rois_size[2];
        int width = tensor_rois_size[3]; 

        tensor_rois = tensor_rois.reshape({-1,4,height,width});

        auto xx = meshes[i].xx;
        auto yy = meshes[i].yy;

        int n = tensor_rois.sizes()[0];

        xx = xx.reshape({1,1,height,width});
        xx = xx.repeat({n,1,1,1});
        yy = yy.reshape({1,1,height,width});
        yy = yy.repeat({n,1,1,1});

        auto rois_width = torch::exp(tensor_rois.index( {torch::indexing::Slice(), 2, torch::indexing::Slice(), torch::indexing::Slice()} )).unsqueeze(1) * scale;
        auto rois_height = torch::exp(tensor_rois.index( {torch::indexing::Slice(), 3, torch::indexing::Slice(), torch::indexing::Slice()} )).unsqueeze(1) * scale;
        auto rois_cx = (tensor_rois.index( {torch::indexing::Slice(), 0, torch::indexing::Slice(), torch::indexing::Slice()} ).unsqueeze(1) * rois_width) + xx;
        auto rois_cy = (tensor_rois.index( {torch::indexing::Slice(), 1, torch::indexing::Slice(), torch::indexing::Slice()} ).unsqueeze(1) * rois_height) + yy;

        tensor_rois = torch::cat( { rois_cx - rois_width*0.5,
                                    rois_cy - rois_height*0.5,
                                    rois_cx + rois_width*0.5,
                                    rois_cy + rois_height*0.5 }
                                , 1 );

        out.push_back( tensor_rois.reshape(tensor_rois_size) );
    }

    return out; 
}

torch::Tensor dnn::tools::permute_flatten_cat( std::vector<torch::Tensor>& tensors )
{
    std::vector<torch::Tensor> ts;

    for( auto &t : tensors )
    {
        int dim = t.sizes()[1];
        t = t.permute({0,2,3,1}).reshape({-1,dim});
        ts.push_back(t);
    }

    auto out = torch::cat( ts, 0 );
    return out;
}

torch::Tensor dnn::tools::scale_rois( torch::Tensor rois, float hscale, float wscale )
{
    rois.index({torch::indexing::Slice(),0}) /= wscale;
    rois.index({torch::indexing::Slice(),1}) /= hscale;
    rois.index({torch::indexing::Slice(),2}) /= wscale;
    rois.index({torch::indexing::Slice(),3}) /= hscale;

    return rois;
}

torch::Tensor dnn::tools::clip_rois( torch::Tensor rois, cv::Size size )
{
    rois.index({torch::indexing::Slice(),0}) = torch::clamp( rois.index({torch::indexing::Slice(),0}), 0, size.width-1 );
    rois.index({torch::indexing::Slice(),1}) = torch::clamp( rois.index({torch::indexing::Slice(),1}), 0, size.height-1 );
    rois.index({torch::indexing::Slice(),2}) = torch::clamp( rois.index({torch::indexing::Slice(),2}), 0, size.width-1 );
    rois.index({torch::indexing::Slice(),3}) = torch::clamp( rois.index({torch::indexing::Slice(),3}), 0, size.height-1 );
    return rois;
}