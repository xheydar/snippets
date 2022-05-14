#include <iostream>
#include <string>
#include <gst/gst.h>
#include <opencv2/opencv.hpp>

using namespace std;

#define IMAGE_HEIGHT 1080
#define IMAGE_WIDTH 1920

static GMainLoop *main_loop;

static gboolean my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err;
      gchar *debug;

      gst_message_parse_error(message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);

      g_main_loop_quit (main_loop);
      break;
    }
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      g_main_loop_quit (main_loop);
      break;
    default:
      /* unhandled message */
      break;
  }

  /* we want to be notified again the next time there is a message
   * on the bus, so returning TRUE (FALSE means we want to stop watching
   * for messages on the bus and our callback should not be called again)
   */
  return TRUE;
}

static GstPadProbeReturn cb_have_data(GstPad *pad,
                                      GstPadProbeInfo *info,
                                      gpointer user_data)
{

    //return GST_PAD_PROBE_OK;
    gint x, y;
    GstMapInfo map;
    guint *ptr, t;
    GstBuffer *buffer;

    buffer = GST_PAD_PROBE_INFO_BUFFER(info);

    buffer = gst_buffer_make_writable(buffer);

    /* Making a buffer writable can fail (for example if it
   * cannot be copied and is used more than once)
   */
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;

    /* Mapping a buffer can fail (non-writable) */
    if (gst_buffer_map(buffer, &map, GST_MAP_WRITE))
    {
        uchar* ptr = reinterpret_cast<uchar*>(map.data);
		cv::Mat image( cv::Size(IMAGE_WIDTH,IMAGE_HEIGHT), CV_8UC3, ptr );
        cv::rectangle( image, cv::Point(0,0), cv::Point(200,200), cv::Scalar(0,0,255), 20 );

        //for( gint i=0 ; i<map.size ; i++ )
        //{
        //    ptr[i] = image.data[i];
        //}

        //cv::rectangle( image, cv::Point(0,0), cv::Point(100,100), cv::Scalar(0,0,255) );
        //memcpy( map.data, dst.data, map.size );
		//cout << image.size() << endl;
        //cv::imshow("Image", image);
        //cv::waitKey(1);
        //cout << map.size / (1920 * 1080)<< endl;

        //cout << static_cast<unsigned int>( map.data[ map.size/2] ) << endl;

        
        /* invert data */
        /*for (y = 0; y < 1080; y++)
        {
            for (x = 0; x < 1920/2; x++)
            {
                gint x0 = y * 1920 + x;
                gint x1 = y * 1920 + (1920 - x - 1);

                t = ptr[ x0 ];
                ptr[ x0 ] = ptr[ x1 ];
                ptr[ x1 ] = t;
            }
            //ptr += 1920;
        }*/
        gst_buffer_unmap(buffer, &map);
    }

    GST_PAD_PROBE_INFO_DATA(info) = buffer;

    return GST_PAD_PROBE_OK;
}

int main( int argc, char** argv )
{
    gst_init(&argc, &argv);
    string source_str = "videotestsrc ! autovideosink";
    
    // Building a pipeline from the string
    GstElement* pipeline = gst_parse_launch( source_str.c_str(), NULL);

    std::cout << pipeline << std::endl;

    if( !pipeline )
    {
        cout << "One of the elements could not be built." << endl;
        gst_object_unref( pipeline );
        return -1;
    }

    // Fetcking the element from the pipeline that we want to do the processing after
    //GstElement* elm = gst_bin_get_by_name(GST_BIN(pipeline), "videoconvert1");
    //if( !elm )
    //{
    //    cout << "Could not get the element" << endl;
    //    gst_object_unref( pipeline );
    //    return -1;
    //}

    // Looking at the frames coming out od this element
    //GstPad *pad = gst_element_get_static_pad (elm, "sink");
    //gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) cb_have_data, NULL, NULL);
    //gst_object_unref (pad);

    /* Start playing */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    guint bus_watch_id = gst_bus_add_watch (bus, my_bus_callback, NULL);

    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);

    gst_object_unref (bus);
    gst_object_unref( pipeline );

    return 0;
}
