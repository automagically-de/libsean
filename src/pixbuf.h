#ifndef _PIXBUF_H
#define _PIXBUF_H

#include <gdk-pixbuf/gdk-pixbuf.h>

guint8 pixbuf_greyscale_r(GdkPixbuf *pixbuf,
	guint32 rx, guint32 ry, guint32 rw, guint32 rh);
gfloat pixbuf_binarize_r(GdkPixbuf *pixbuf, guint8 threshold,
	guint32 rx, guint32 ry, guint32 rw, guint32 rh);

void pixbuf_median_r(GdkPixbuf *pixbuf);
guint8 pixbuf_average_r(GdkPixbuf *pixbuf,
	guint32 rx, guint32 ry, guint32 rw, guint32 rh);

void pixbuf_r_to_gb(GdkPixbuf *pixbuf);

#endif /* _PIXBUF_H */
