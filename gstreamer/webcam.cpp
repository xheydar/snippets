#include <iostream>
#include <gst/gst.h>
#include <opencv2/opencv.hpp>

using namespace std;

#define IMAGE_HEIGHT 1080
#define IMAGE_WIDTH 1920

struct CustomData
{
    GstElement *pipeline;
    GstElement *source;
    GstElement *filter1; // Changing the resolution to get 1080p feed
    GstElement *convert;
    GstElement *filter2; // Changing the color space to be better for opencv
                         // We will use the output of this filter for image processing
    GstElement *sink_convert;
    GstElement *sink; 
};

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

    CustomData data;

    data.source = gst_element_factory_make("autovideosrc", "source");
    //data.source = gst_element_factory_make("uridecodebin", "source");
    data.filter1 = gst_element_factory_make("capsfilter", "filter1");
    data.convert = gst_element_factory_make("videoconvert", "convert");
    data.filter2 = gst_element_factory_make("capsfilter", "filter2");
    data.sink = gst_element_factory_make("autovideosink", "sink");
    //data.sink = gst_element_factory_make("fakesink", "sink");
    //data.sink_convert = gst_element_factory_make("videoconvert", "sink_convert");
    //data.sink = gst_element_factory_make("tcpserversink", "sink");
    if( !data.source || !data.filter1 || !data.convert || !data.filter2 || !data.sink )
    {
        g_printerr("Could not create some of the elements\n");
        return -1;
    }

    // How to set the device???
    // g_object_set(G_OBJECT(data.source), "device", "/dev/video0", NULL );

    data.pipeline = gst_pipeline_new("StreamPipeline");
    if( !data.pipeline )
    {
        g_printerr("Could not create pipeline\n");
        return -1;
    }

    gst_bin_add_many( GST_BIN(data.pipeline), data.source, data.filter1, data.convert, data.filter2, data.sink, NULL );
    if( !gst_element_link_many(data.source, data.filter1, data.convert, data.filter2, data.sink, NULL) )
    {
        g_printerr("Some elements could not be linked.\n");
        gst_object_unref( data.pipeline );
        return -1;
    }

    //g_object_set( data.source, "device=/dev/video0", NULL );

    GstCaps *filtercaps;
    
    filtercaps = gst_caps_new_simple("video/x-raw", 
                                              //"format", G_TYPE_STRING, "BGR", 
                                              "width", G_TYPE_INT, IMAGE_WIDTH,
                                              "height", G_TYPE_INT, IMAGE_HEIGHT,
                                              NULL);
    g_object_set(G_OBJECT(data.filter1), "caps", filtercaps, NULL);
    gst_caps_unref(filtercaps);

    filtercaps = gst_caps_new_simple("video/x-raw", 
                                     "format", G_TYPE_STRING, "BGR",
                                     NULL);

    g_object_set(G_OBJECT(data.filter2), "caps", filtercaps, NULL);
    gst_caps_unref(filtercaps);

    g_object_set(G_OBJECT(data.sink), "sync", 1, NULL);

    GstPad *pad = gst_element_get_static_pad (data.filter2, "sink");
    gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) cb_have_data, NULL, NULL);
    gst_object_unref (pad);

    /* Start playing */
    gst_element_set_state (data.pipeline, GST_STATE_PLAYING);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(data.pipeline));
    guint bus_watch_id = gst_bus_add_watch (bus, my_bus_callback, NULL);

    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);

    gst_object_unref (bus);
    gst_object_unref( data.pipeline );

    return 0;
}
