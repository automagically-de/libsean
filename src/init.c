#include <glib.h>

#include <sean.h>

#include "video.h"
#include "scan.h"

SeanContext *sean_init(guint32 flags, int *argcp, char ***argvp)
{
	SeanContext *context;

	context = g_new0(SeanContext, 1);
	context->binarize_threshold = 64;

	if(flags & SEAN_USE_GSTREAMER)
	{
		video_init(context, argcp, argvp);
		g_idle_add((GSourceFunc)scan_idle_cb, context);
	}

	return context;
}

void sean_close(SeanContext *context)
{
	g_free(context);
}

void sean_register_preview_callback(SeanContext *context,
	SeanPreviewFunc callback, gpointer user_data)
{
	g_assert(context);
	context->preview_callback = callback;
	context->preview_data = user_data;
}

void sean_register_success_callback(SeanContext *context,
	SeanSuccessFunc callback, gpointer user_data)
{
	g_assert(context);
	context->success_callback = callback;
	context->success_data = user_data;
}

void sean_set_binarize_threshold(SeanContext *context, guint8 threshold)
{
	g_assert(context);
	context->binarize_threshold = threshold;
}

void sean_set_filter(SeanContext *context, guint32 filter)
{
	g_assert(context);
	context->filter = filter;
}
