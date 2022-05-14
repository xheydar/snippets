#include <iostream>
#include <string>
#include <gst/gst.h>
#include <opencv2/opencv.hpp>

using namespace std;

static GMainLoop *main_loop;

int main( int argc, char** argv )
{
    gst_init(&argc, &argv);
    string source_str = "videotestsrc ! autovideosink";

    GError *err = NULL;
    // Building a pipeline from the string
    GstElement* pipeline = gst_parse_launch( source_str.c_str(), &err);

    std::cout << pipeline << std::endl;

    if( err != NULL )
    {
        std::cout << err->message << std::endl;
        g_clear_error (&err);
    }  

    if( !pipeline )
    {
        cout << "One of the elements could not be built." << endl;
        gst_object_unref( pipeline );
        return -1;
    }

    /* Start playing */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);

    gst_object_unref( pipeline );

    return 0;
}
