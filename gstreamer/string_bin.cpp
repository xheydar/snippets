#include <iostream>
#include <gst/gst.h>
#include <vector>
#include <thread>
#include <opencv2/opencv.hpp>

using namespace std;

static GMainLoop *main_loop;
unsigned int count_num = 0;

struct Data
{
    GstElement *pipeline;
    GstElement *source;
    GstElement *convert;
    GstElement *filter;
    GstElement *delay;
    GstElement *sink;
};

struct Frame
{
    cv::Mat image;
    unsigned int id;
    GstClockTime timestamp;
};

vector<Frame> process_queue;
vector<Frame> results_queue;

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

static GstPadProbeReturn call_back_extraction(GstPad *pad,
                                              GstPadProbeInfo *info,
                                              gpointer user_data)
{

    //return GST_PAD_PROBE_OK;
    gint x, y;
    GstMapInfo map;
    guint *ptr, t;
    GstBuffer *buffer;

    //cout << "Extraction" << "\t" << info -> id << "\t" << info -> offset << "\t" << info -> type << endl;

    buffer = GST_PAD_PROBE_INFO_BUFFER(info);

    buffer = gst_buffer_make_writable(buffer);

    /* Making a buffer writable can fail (for example if it
   * cannot be copied and is used more than once)
   */
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;

    bool has_output = false;

    /* Mapping a buffer can fail (non-writable) */
    if (gst_buffer_map(buffer, &map, GST_MAP_WRITE))
    {
        uchar* ptr = reinterpret_cast<uchar*>(map.data);
		cv::Mat image( cv::Size(1920,1080), CV_8UC3, ptr );
        //cv::rectangle( image, cv::Point(0,0), cv::Point(200,200), cv::Scalar(0,0,255), 20 );

        Frame frame;
        image.copyTo( frame.image );
        frame.id = count_num ++;
        frame.timestamp = buffer -> pts; 
        process_queue.push_back( frame );

        GstClockTime delay = buffer -> pts - process_queue[0].timestamp;
        double elapsed = static_cast<double>( delay ) / 1000000000;

        if( elapsed >= 1.0 )
        {
            Frame frame = process_queue[0];
            process_queue.erase( process_queue.begin() ); 

            cv::rectangle( frame.image, cv::Point(0,0), cv::Point(200,200), cv::Scalar(0,0,255), 20 );

            cout << buffer -> pts - frame.timestamp << endl;
            
            unsigned int *ptr_uint = reinterpret_cast<unsigned int*>(ptr);
            memcpy(ptr, frame.image.data, map.size);
            has_output = true;
        }

        gst_buffer_unmap(buffer, &map);
    }

    cout << "Queue size : " << process_queue.size() << endl;

    GST_PAD_PROBE_INFO_DATA(info) = buffer;

    if( has_output )
    {
        return GST_PAD_PROBE_OK;
    }
    else
    {
        return GST_PAD_PROBE_DROP;
    }
    //return GST_PAD_PROBE_DROP;
    //return GST_PAD_PROBE_REMOVE;
}

int main( int argc, char** argv )
{
    gst_init( &argc, &argv );

    string source_desc = "autovideosrc device=/dev/video0 ! video/x-raw,width=1920,height=1080 ! videoconvert";
    string delay_desc = "queue max-size-buffers=0 max-size-time=0 max-size-bytes=0 min-threshold-time=50000000";
    //string sink_desc = "clockoverlay shaded-background=true font-desc=\"Sans 38\" ! autovideosink";
    //string sink_desc = "fakesink";
    string sink_desc = "videoconvert ! videoconvert ! clockoverlay ! x264enc tune=zerolatency ! mpegtsmux ! hlssink playlist-root=http://127.0.0.1:8000 location=./segment_%05d.ts target-duration=1 max-files=5";

    Data data;
    GError *error = NULL;

    data.source = gst_parse_bin_from_description( source_desc.c_str(), TRUE, &error );
    data.convert = gst_element_factory_make("videoconvert", "process_convert");
    data.filter = gst_element_factory_make("capsfilter", "process_filter");
    data.delay = gst_parse_bin_from_description( delay_desc.c_str(), TRUE, &error );
    data.sink = gst_parse_bin_from_description( sink_desc.c_str(), TRUE, &error );

    if( !data.source || !data.convert || !data.filter || !data.delay || !data.sink )
    {
        cout << "Could not load one of the elements!" << endl;
        return -1;
    }

    data.pipeline = gst_pipeline_new("StreamPipeline");
    if( !data.pipeline )
    {
        g_printerr("Could not create pipeline\n");
        return -1;
    }

    gst_bin_add_many( GST_BIN(data.pipeline), data.source, data.convert, data.filter, data.delay, data.sink, NULL );
    if( !gst_element_link_many(data.source, data.convert, data.filter, data.delay, data.sink, NULL) )
    {
        g_printerr("Some elements could not be linked.\n");
        gst_object_unref( data.pipeline );
        return -1;
    }

    GstCaps *filtercaps;

    filtercaps = gst_caps_new_simple("video/x-raw", 
                                     "format", G_TYPE_STRING, "BGR",
                                     NULL);
    g_object_set(G_OBJECT(data.filter), "caps", filtercaps, NULL);
    gst_caps_unref(filtercaps);

    // Looking at the frames coming out od this element
    GstPad *pad;

    pad = gst_element_get_static_pad (data.filter, "sink");
    gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) call_back_extraction, NULL, NULL);
    gst_object_unref (pad);

    //pad = gst_element_get_static_pad (data.delay, "sink");
    //gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) call_back_injection, NULL, NULL);
    //gst_object_unref (pad);

    gst_element_set_state (data.pipeline, GST_STATE_PLAYING);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(data.pipeline));
    guint bus_watch_id = gst_bus_add_watch (bus, my_bus_callback, NULL);

    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);

    gst_object_unref (bus);
    gst_object_unref( data.pipeline );

    return 0;
}