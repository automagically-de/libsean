#include <string.h>

#include <sean.h>

#include "bar.h"
#include "pixbuf.h"

#define OPTIMAL_BLACK_PCNT 39.0
#define ADJUST_THR_STEP 0.2

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

gboolean sean_scan_pixbuf(SeanContext *context, GdkPixbuf *pixbuf,
	guint32 *code)
{
	guint32 x, y, i, rw, rh, w, h;
	guint8 *thr;
#define nyvals 13
	gfloat yvals[] = { 0.5, 0.45, 0.55,
		0.4, 0.6, 0.35, 0.65, 0.3, 0.7, 0.25, 0.75, 0.2, 0.8 };
/* #define DUMP_PIXBUF 1 */
#ifdef DUMP_PIXBUF
	static guint32 dump_index = 0;
	gchar *filename;

	filename = g_strdup_printf("dump_pixbuf_%08u.bmp", dump_index ++);
	gdk_pixbuf_save(pixbuf, filename, "bmp", NULL, NULL);
#endif

	w = gdk_pixbuf_get_width(pixbuf);
	h = gdk_pixbuf_get_height(pixbuf);
	rw = rh = 256;
	x = y = i = 0;
	thr = g_new0(guint8, (w / rw + 1) * (h / rh + 1));
	while((x + 1) < w)
	{
		y = 0;
		while((y + 1) < h)
		{
			thr[i] = pixbuf_greyscale_r(pixbuf, x, y,
				min(rw, w - x), min(rh, h - y));
			if(context->filter & SEAN_FILTER_AVERAGE)
				pixbuf_average_r(pixbuf, x, y,
					min(rw, w - x), min(rh, h - y));
			i ++;
			y += rh;
		}
		x += rw;
	}

	if(context->filter & SEAN_FILTER_MEDIAN)
		pixbuf_median_r(pixbuf);

	x = y = i = 0;
	while((x + 1) < w)
	{
		y = 0;
		while((y + 1) < h)
		{
			pixbuf_binarize_r(pixbuf, thr[i], x, y,
				min(rw, w - x), min(rh, h - y));
			i ++;
			y += rh;
		}
		x += rw;
	}
	g_free(thr);

	pixbuf_r_to_gb(pixbuf);

	/* scan code */
	for(y = 0; y < nyvals; y ++)
	{
		if(bar_read_code(gdk_pixbuf_get_pixels(pixbuf) +
			((gint)(gdk_pixbuf_get_height(pixbuf) * yvals[y]) *
			gdk_pixbuf_get_rowstride(pixbuf)),
			gdk_pixbuf_get_width(pixbuf), code))
		{
			if(context->success_callback)
				context->success_callback(code, context->success_data);
			return TRUE;
		}
	}

	return FALSE;
}

gboolean scan_idle_cb(SeanContext *context)
{
	GdkPixbuf *scaled, *pixbuf;
	guint32 code[14];

	g_assert(context);

	pixbuf = context->pixbuf;
	if(pixbuf == NULL)
	{
		g_usleep(1000);
		return TRUE;
	}

	/* try to read code from image */
	sean_scan_pixbuf(context, pixbuf, code);

	/* resize and display preview image */
	scaled = gdk_pixbuf_scale_simple(pixbuf, 320, 240, GDK_INTERP_BILINEAR);
	if(scaled)
	{
		if(context->preview_callback)
			context->preview_callback(scaled, context->preview_data);

		gdk_pixbuf_unref(scaled);
	}

	gdk_pixbuf_unref(pixbuf);
	context->pixbuf = NULL;

	g_usleep(1000);
	return TRUE;
}
