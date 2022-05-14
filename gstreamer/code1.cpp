#include <gst/gst.h>

static GMainLoop *main_loop;

static gboolean my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
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

int main( int argc, char** argv )
{
    GstElement* pipeline, *source, *filter, *overlay, *sink;
    GstBus* bus;
    guint bus_watch_id;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    source = gst_element_factory_make("videotestsrc","source");
    if( !source )
    {
        g_printerr("Couldn't create source.\n");
        return -1; 
    }

    filter = gst_element_factory_make("vertigotv","filter");
    if( !filter )
    {
        g_printerr("Couldn't create filter.\n");
        return -1; 
    }

    /*overlay = gst_element_factory_make("fpsdisplaysink","overlay");
    if( !overlay )
    {
        g_printerr("Couldn't create overlay.\n");
        return -1; 
    }*/

    sink = gst_element_factory_make("autovideosink","sink");
    if( !sink )
    {
        g_printerr("Couldn't create sink.\n");
        return -1; 
    }

    pipeline = gst_pipeline_new("test-pipeline");
    if( !sink )
    {
        g_printerr("Couldn't create pipeline.\n");
        return -1; 
    }

    gst_bin_add_many( GST_BIN(pipeline), source, filter, sink, NULL );
    if( gst_element_link_many( source, filter, sink, NULL ) != TRUE )
    {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    g_object_set( source, "pattern", 0, NULL );
    ret = gst_element_set_state( pipeline, GST_STATE_PLAYING );
    if( ret == GST_STATE_CHANGE_FAILURE )
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline);
        return -1;
    }
    
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    bus_watch_id = gst_bus_add_watch (bus, my_bus_callback, NULL);
    

    main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);

    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
    return 0;
}