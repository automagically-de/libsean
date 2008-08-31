#ifndef _SEAN_H
#define _SEAN_H

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gst/gst.h>

typedef void (*SeanPreviewFunc)(GdkPixbuf *image, gpointer user_data);
typedef void (*SeanSuccessFunc)(guint32 *code, gpointer user_data);

typedef struct {
	/* image processing stuff */
	GdkPixbuf *pixbuf;
	guint8 binarize_threshold;
	guint32 filter;

	/* preview stuff */
	SeanPreviewFunc preview_callback;
	gpointer preview_data;

	/* success (code received) stuff */
	SeanSuccessFunc success_callback;
	gpointer success_data;

	/* GStreamer stuff */
	GstElement *pipeline;
	GstBus *bus;

} SeanContext;

#define SEAN_USE_GSTREAMER    (1 << 0)

#define SEAN_FILTER_NONE      0x00
#define SEAN_FILTER_MEDIAN    0x01
#define SEAN_FILTER_AVERAGE   0x02

SeanContext *sean_init(guint32 flags, int *argcp, char ***argvp);
void sean_close(SeanContext *context);
void sean_register_preview_callback(SeanContext *context,
	SeanPreviewFunc callback, gpointer user_data);
void sean_register_success_callback(SeanContext *context,
	SeanSuccessFunc callback, gpointer user_data);
void sean_set_binarize_threshold(SeanContext *context, guint8 threshold);
void sean_set_filter(SeanContext *context, guint32 filter);

/* video */
gboolean sean_video_play(SeanContext *context);
gboolean sean_video_pause(SeanContext *context);

/* scan */
gboolean sean_scan_pixbuf(SeanContext *context, GdkPixbuf *pixbuf,
	guint32 *code);

#endif
