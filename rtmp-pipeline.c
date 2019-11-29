#include <stdio.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

typedef struct _App {
  GstElement *rtmpsrc;
  GstElement *h264parse, *playbin;
  GstElement *videocoverter, *avdec_h264, *xvimagesink, *rtmpsink;
  GMainLoop *loop;
} App;

static void analyze_streams(App* data)
{
        g_print ("analyze_streams .\n");
}
static void cb_message (GstBus *bus, GstMessage *msg, App *data) {

  switch (GST_MESSAGE_TYPE (msg)) {

    default:
      /* Unhandled message */
        g_print ("End-Of-Stream reached.\n");
      break;
    }
}

static gboolean bus_call (GstBus *bus, GstMessage *msg, App *data) {
  GError *err;
  gchar *debug_info;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error (msg, &err, &debug_info);
      g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
      g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
      g_clear_error (&err);
      g_free (debug_info);
      g_main_loop_quit (data->loop);
      break;
    case GST_MESSAGE_EOS:
      g_print ("End-Of-Stream reached.\n");
      g_main_loop_quit (data->loop);
      break;
    case GST_MESSAGE_STATE_CHANGED: {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
//       if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->playbin)) {
//         if (new_state == GST_STATE_PLAYING) {
//           /* Once we are in the playing state, analyze the streams */
//           analyze_streams (data);
//         }
//       }
    } break;
  }

  /* We want to keep receiving messages */
  return TRUE;
}

int
main (int argc, char *argv[])
{
    App app; 
    GstFlowReturn ret;
    GstBus *bus;
    GstElement *pipeline;

    gst_init (NULL,NULL);

    GMainLoop *loop = g_main_loop_new (NULL, TRUE);

    //creazione della pipeline:
    pipeline = gst_pipeline_new ("gstreamer-encoder");
    if( ! pipeline ) {
        g_print("Error creating Pipeline, exiting...");
    }

    //creazione elemento appsrc:
    app.rtmpsrc = gst_element_factory_make ("rtmpsrc", "rtmpsrc");
    if( !  app.rtmpsrc ) {
            g_print( "Error creating source element, exiting...");
    }

    //creazione elemento h264parse:
    app.h264parse = gst_element_factory_make ("h264parse", "h264parse");
    if( !  app.h264parse ) {
            g_print( "Error creating h264parse element, exiting...");
    }

    app.videocoverter = gst_element_factory_make ("videoconvert", "videocoverter");
    if(!app.videocoverter ) {
            g_print( "Error creating videocoverter, exiting...");
    }


    app.avdec_h264 = gst_element_factory_make ("avdec_h264", "avdec_h264");
    if(!app.avdec_h264 ) {
            g_print( "Error creating omxh264enc, exiting...");
    }

    app.xvimagesink = gst_element_factory_make ("xvimagesink", "xvimagesink");
    if( !app.xvimagesink ) {
            g_print( "Error creating xvimagesink, exiting...");
    }



    g_print ("Elements are created\n");
    g_object_set (G_OBJECT (app.rtmpsrc), "location" , "rtmp://127.0.0.1:1935/live/livestream" ,  NULL);
    


    g_print ("end of settings\n");

    bus = gst_pipeline_get_bus (GST_PIPELINE ( pipeline));
    g_assert(bus);
    gst_bus_add_watch ( bus, (GstBusFunc)bus_call, &app);

    gst_bin_add_many (GST_BIN (pipeline), app.rtmpsrc, app.h264parse, app.avdec_h264, app.videocoverter,app.xvimagesink, NULL);

    g_print ("Added all the Elements into the pipeline\n");

    int ok = FALSE;
    ok = gst_element_link_many (app.rtmpsrc, app.h264parse, app.avdec_h264, app.videocoverter, app.xvimagesink, NULL);


    if(ok)g_print ("Linked all the Elements together\n");
    else g_print("*** Linking error ***\n");

    g_assert(app.rtmpsrc);



    g_print ("Playing the video\n");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    g_print ("Running...\n");
        g_main_loop_run ( loop);

    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref ( bus);
    g_main_loop_unref (loop);
    g_print ("Deleting pipeline\n");
}