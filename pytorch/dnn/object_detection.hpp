#ifndef DNN_OBJECT_DETECTION_HPP
#define DNN_OBJECT_DETECTION_HPP

#include <iostream>
#include <string>
#include <map>
#include <opencv2/opencv.hpp>
#include <memory>
#include <chrono>
#include <yaml-cpp/yaml.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <torch/script.h>
#include <torch/torch.h>
#include <torchvision/ops/nms.h>
#pragma clang diagnostic pop

#include "./tools.hpp"

namespace dnn
{
    struct tensor_scale
    {
        cv::Size original_size;
        cv::Size size;
        float hscale;
        float wscale;
    };

    struct detections
    {
        int cls_label;
        std::string cls_name;
        torch::Tensor scores;
        torch::Tensor rois;
    };

    class object_detection
    {
        private:
            torch::jit::script::Module module;
            int max_side;

            float score_thresh = 0.05;
            float nms_thresh = 0.5;
            bool unified_rois = true;

            std::vector<float> mean = {0.485, 0.456, 0.406};
            std::vector<float> std = {0.229, 0.224, 0.225};

            std::map<int,std::string> class_names;

            //std::vector<tools::mesh> mesh_grids_cache;

            std::tuple<torch::Tensor, tensor_scale> preprocess( cv::Mat );   
            std::vector<detections> postprocess( std::vector<torch::Tensor>&, 
                                                 std::vector<torch::Tensor>&, 
                                                 tensor_scale );         
        public:
            object_detection( YAML::Node );
            //object_detection( std::string, int ); // Model path, Max side
            std::vector<detections> process( cv::Mat );
    };
}

#endif
