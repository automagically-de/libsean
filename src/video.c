#include <string.h>

#include <gst/gst.h>

#include <sean.h>

#define PIPELINE "v4l2src name=source " \
 "! video/x-raw-rgb,bpp=24,width=720,height=578 " \
 "! fakesink signal-handoffs=true name=dest"

gboolean video_on_message_cb(GstBus * bus, GstMessage * message, gpointer data)
{
	GstState state;
	GError *error;

	switch(GST_MESSAGE_TYPE(message))
	{
		case GST_MESSAGE_STATE_CHANGED:
			gst_message_parse_state_changed(message, NULL, &state, NULL);
			break;

		case GST_MESSAGE_EOS:
			g_warning("end of stream\n");
			break;

		case GST_MESSAGE_ERROR:
			gst_message_parse_error(message, &error, NULL);
			g_warning("GST error: %s\n", error->message);
			g_free(error);
			break;

		default:
			break;
	}

	return TRUE;
}

void video_free_frame(guchar *pixels, gpointer data)
{
	g_free(pixels);
}

gboolean video_on_frame_cb(GstElement *element, GstBuffer *buffer,
	GstPad *pad, SeanContext *context)
{
	GdkPixbuf *pixbuf;
	static GstStructure *caps = NULL;
	static int width, height, bpp, depth;
	guchar *pixels;

	if(caps == NULL)
	{
		/* init caps */
		caps = gst_caps_get_structure(GST_BUFFER_CAPS(buffer), 0);
		gst_structure_get_int(caps, "width", &width);
		gst_structure_get_int(caps, "height", &height);
		gst_structure_get_int(caps, "bpp", &bpp);
		gst_structure_get_int(caps, "depth", &depth);
	}

	pixels = g_new0(guchar, width * height * 3);
	memcpy(pixels, GST_BUFFER_DATA(buffer), sizeof(guchar) * width*height*3);

	pixbuf = gdk_pixbuf_new_from_data(pixels, GDK_COLORSPACE_RGB, FALSE, 8,
		width, height, width * 3,
		video_free_frame, NULL);

	if(pixbuf)
	{
		if(context->pixbuf == NULL)
			context->pixbuf = pixbuf;
		else
			gdk_pixbuf_unref(pixbuf);
	}

	return TRUE;
}

gboolean video_init(SeanContext *context, int *argcp, char ***argvp)
{
	GstElement *dest;
	GError *error = NULL;

	gst_init(argcp, argvp);

	context->pipeline = gst_parse_launch(PIPELINE, &error);
	if(context->pipeline == NULL)
	{
		g_warning("GST init error: %s\n", error->message);
		g_error_free(error);
		return FALSE;
	}

	if(error)
	{
		g_warning("GST init warning: %s\n", error->message);
		g_error_free(error);
	}

	context->bus = gst_pipeline_get_bus(GST_PIPELINE(context->pipeline));

	gst_element_set_state(context->pipeline, GST_STATE_PAUSED);

	gst_bus_add_signal_watch(context->bus);
	g_signal_connect(context->bus, "message", G_CALLBACK(video_on_message_cb),
		NULL);

	dest = gst_bin_get_by_name(GST_BIN(context->pipeline), "dest");
	g_signal_connect(dest, "handoff", G_CALLBACK(video_on_frame_cb), context);

#if DEBUG > 0
	g_print("SEAN: GST initialization done\n");
#endif

	return TRUE;
}

gboolean sean_video_play(SeanContext *context)
{
	return (gst_element_set_state(context->pipeline, GST_STATE_PLAYING)
		!= GST_STATE_CHANGE_FAILURE);
}

gboolean sean_video_pause(SeanContext *context)
{
	if(context->pixbuf)
	{
		gdk_pixbuf_unref(context->pixbuf);
		context->pixbuf = NULL;
	}
	return (gst_element_set_state(context->pipeline, GST_STATE_PAUSED)
		!= GST_STATE_CHANGE_FAILURE);
}

