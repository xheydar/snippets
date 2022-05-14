#ifndef DNN_TOOLS_HPP
#define DNN_TOOLS_HPP

#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <torch/torch.h>
#pragma clang diagnostic pop

namespace dnn::tools
{
    struct mesh
    {
        torch::Tensor xx;
        torch::Tensor yy;
        torch::Tensor x_linspace;
        torch::Tensor y_linspace;
        double scale;
    };

    cv::Vec3f normalize_pixel( cv::Vec3f pixel, cv::Vec3f mean, cv::Vec3f std );
    void normalize_image( cv::Mat img_float, cv::Vec3d mean, cv::Vec3f std );
    double calculate_scale( cv::Size s, int max_side );

    std::vector<mesh> build_meshgrid( std::vector<torch::Tensor>&, int );
    std::vector<torch::Tensor> map_bbox_regression( std::vector<torch::Tensor>&, std::vector<mesh>& );

    torch::Tensor permute_flatten_cat( std::vector<torch::Tensor>& );
    torch::Tensor scale_rois( torch::Tensor, float, float );
    torch::Tensor clip_rois( torch::Tensor, cv::Size );
}

#endif
