#include <iostream>
#include <tuple>
#include <vector>
#include <memory>
#include <map>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <yaml-cpp/yaml.h>

#include "dnn/object_detection.hpp"

int main( int argc, char** argv )
{
    YAML::Node config = YAML::LoadFile("../config.yml");
    dnn::object_detection object_detection( config["detection"] );
    
    cv::Mat img = cv::imread("../000000386912.jpg");
    auto t0 = std::chrono::steady_clock::now();
    auto detections = object_detection.process( img.clone() );
    auto t1 = std::chrono::steady_clock::now();

    std::map<int,cv::Scalar> colors;
    colors[0] = cv::Scalar(0,0,255);
    colors[1] = cv::Scalar(255,0,0);

    for( auto &dets : detections )
    {
        std::cout << dets.cls_label << "\t" << dets.cls_name << std::endl;
        int num = dets.scores.sizes()[0];

        for( int i=0 ; i<num; i++ )
        {
            float s = dets.scores[i].item<float>();
            auto roi = dets.rois[i];
            int x0 = std::round( roi[0].item<float>() );
            int y0 = std::round( roi[1].item<float>() );
            int x1 = std::round( roi[2].item<float>() );
            int y1 = std::round( roi[3].item<float>() );

            cv::Rect r(x0,y0,x1-x0,y1-y0);
            cv::rectangle( img, r, colors[dets.cls_label], 1 );
        }
    }

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>( t1 - t0 ).count() << std::endl;
    //std::cout << scores.sizes() << "\t" << rois.sizes() << std::endl;

    cv::imshow("Image", img);
    cv::waitKey();
    

    return 0;
}
