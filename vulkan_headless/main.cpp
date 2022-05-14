#include <iostream>
#include <fstream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <thread>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

#include "vengine/module.hpp"


using namespace std;

double clockToMilliseconds(clock_t ticks){
    // units/(units/time) => time (seconds) * 1000 = milliseconds
    return (ticks/(double)CLOCKS_PER_SEC)*1000.0;
}

void blend_overlay( cv::Mat &src, cv::Mat &overlay )
{
    int total = src.total();

    uchar* src_ptr = src.data;
    uchar* overlay_ptr = overlay.data;

    for( int i=0 ; i<total ; i++ )
    {
        if( overlay_ptr[3] != 0 )
        {
            memcpy( src_ptr, overlay_ptr, 3 );
        }

        src_ptr += src.elemSize();
        overlay_ptr += overlay.elemSize();;
    }
}

int main( int argc, char** argv )
{
    std::ifstream infile("../config.json");
    nlohmann::json config_json = nlohmann::json::parse( infile );

    clock_t deltaTime = 0;
    unsigned int frames = 0;
    double  frameRate = 30;
    double  averageFrameTimeMilliseconds = 33.333;

    try
    {
        vengine::module module( config_json["assets"], {1920,1080} );

#ifdef USE_WEBCAM
        cv::namedWindow("Image",1);
        cv::VideoCapture cap(0);
        assert( cap.isOpened() );

        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
        cap.set(cv::CAP_PROP_FPS, 30);
#endif

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        while( true )
        {
#ifdef USE_WEBCAM
            cv::Mat src;
            cap >> src;
            if( !src.empty() )
#endif
            {
                clock_t beginFrame = clock();
                auto overlay = module.render();
                clock_t endFrame = clock();

                deltaTime += endFrame - beginFrame;
                std::cout << "Per frame render time" 
                    << "\t" << static_cast<double>(endFrame - beginFrame)/1000 
                    << " miliseconds" << std::endl;
                frames ++;
                //if you really want FPS
                if( clockToMilliseconds(deltaTime)>1000.0){ //every second
                    frameRate = (double)frames*0.5 +  frameRate*0.5; //more stable
                    frames = 0;
                    deltaTime -= CLOCKS_PER_SEC;
                    averageFrameTimeMilliseconds  = 1000.0/(frameRate==0?0.001:frameRate);

                    std::cout<<"Average Render Time:"<<averageFrameTimeMilliseconds<<std::endl;
                }

                cv::cvtColor( overlay, overlay, cv::COLOR_BGRA2RGBA );
#ifdef USE_WEBCAM
                //clock_t t0 = clock();
                blend_overlay( src, overlay );
                //clock_t t1 = clock();

                //std::cout << static_cast<double>(t1 - t0)/1000 << std::endl;
                cv::imshow("Image", src);
                cv::waitKey(1);
#else
                cv::imshow("Image", overlay);
                cv::waitKey(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

#endif
            }
        }
    }
    catch(std::exception &e)
    {
        std::cerr << "[ Error ] - " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
