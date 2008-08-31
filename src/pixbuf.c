#include <string.h>
#include <stdlib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

static inline guint32 minn(guint32 *v, guint8 n)
{
	guint8 i;
	guint32 val = 0xFFFFFFFF;

	for(i = 0; i < n; i ++)
		if(v[i] < val)
			val = v[i];
	return val;
}

static guint8 get_auto_threshold(guint32 *histogram)
{
	guint32 maxv, maxi, i, minv;

	maxv = 0;
	maxi = 10;

	/* get maximum value (white peak) */
	for(i = 10; i < 256; i ++)
	{
		if(histogram[i] >= maxv)
		{
			maxv = histogram[i];
			maxi = i;
		}
	}

	/* get minimum left of peak */
	i = maxi;
	minv = maxv;
#define KWIDTH 5
	while(i >= (KWIDTH - 1))
	{
		if(minn(histogram + i - (KWIDTH - 1), KWIDTH) <= minv)
		{
			minv = minn(histogram + i - (KWIDTH - 1), KWIDTH);
			i --;
		}
		else return i;
	}

	return maxi;
}

guint8 pixbuf_greyscale_r(GdkPixbuf *pixbuf,
	guint32 rx, guint32 ry, guint32 rw, guint32 rh)
{
	guint8 *pixels;
	guint32 histogram[256];
	guint32 width, height, x, y, i, rowstride, nchannels;

	pixels = gdk_pixbuf_get_pixels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	nchannels = gdk_pixbuf_get_n_channels(pixbuf);

	/* clear histogram */
	for(i = 0; i < 256; i++)
		histogram[i] = 0;

	for(y = ry; y < (ry + rh); y ++)
		for(x = rx; x < (rx + rw); x ++)
		{
			i = y * rowstride + x * nchannels;
			pixels[i] =
				0.299 * pixels[i] +
				0.587 * pixels[i + 1] +
				0.114 * pixels[i + 2];
			histogram[pixels[i]] ++;
		}

	return get_auto_threshold(histogram);
}

void pixbuf_r_to_gb(GdkPixbuf *pixbuf)
{
	guint8 *pixels;
	guint32 width, height, x, y, i, rowstride, nchannels;

	pixels = gdk_pixbuf_get_pixels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	nchannels = gdk_pixbuf_get_n_channels(pixbuf);

	for(y = 0; y < height; y ++)
		for(x = 0; x < width; x ++)
		{
			i = y * rowstride + x * nchannels;
			pixels[i + 1] = pixels[i + 2] = pixels[i];
		}
}

gfloat pixbuf_binarize_r(GdkPixbuf *pixbuf, guint8 threshold,
	guint32 rx, guint32 ry, guint32 rw, guint32 rh)
{
	guint8 *pixels;
	guint32 width, height, x, y, i, rowstride, nchannels;
	guint32 sum_black;

	sum_black = 0;
	pixels = gdk_pixbuf_get_pixels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	nchannels = gdk_pixbuf_get_n_channels(pixbuf);

	for(y = ry; y < (ry + rh); y ++)
		for(x = rx; x < (rx + rw); x ++)
		{
			i = y * rowstride + x * nchannels;
			if(pixels[i] > threshold)
				pixels[i] = 0xFF;
			else
			{
				pixels[i] = 0x00;
				sum_black ++;
			}
		}

	return (gfloat)sum_black / (gfloat)(rw * rh) * 100.0;
}

/* http://ndevilla.free.fr/median/median.pdf */
#define PIX_SORT(a, b) { if((a) > (b)) PIX_SWAP((a), (b)); }
#define PIX_SWAP(a, b) { guint8 temp=(a); (a) = (b); (b) = temp; }

static guint8 sort_a9(guint8 *p)
{
	PIX_SORT(p[1], p[2]); PIX_SORT(p[4], p[5]); PIX_SORT(p[7], p[8]);
	PIX_SORT(p[0], p[1]); PIX_SORT(p[3], p[4]); PIX_SORT(p[6], p[7]);
	PIX_SORT(p[1], p[2]); PIX_SORT(p[4], p[5]); PIX_SORT(p[7], p[8]);
	PIX_SORT(p[0], p[3]); PIX_SORT(p[5], p[8]); PIX_SORT(p[4], p[7]);
	PIX_SORT(p[3], p[6]); PIX_SORT(p[1], p[4]); PIX_SORT(p[2], p[5]);
	PIX_SORT(p[4], p[7]); PIX_SORT(p[4], p[2]); PIX_SORT(p[6], p[4]);
	PIX_SORT(p[4], p[2]); return(p[4]);
}

void pixbuf_median_r(GdkPixbuf *pixbuf)
{
	guint8 *pixels, *pcopy;
	guint32 width, height, i, rowstride, nchannels;
	gint32 x, y, xo, yo;
	guint8 avals[9], aptr;

	pixels = gdk_pixbuf_get_pixels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	nchannels = gdk_pixbuf_get_n_channels(pixbuf);

	pcopy = g_new0(guint8, height * rowstride);
	memcpy(pcopy, pixels, height * rowstride * sizeof(guint8));

	for(y = 1; y < (height - 1); y ++)
		for(x = 1; x < (width - 1); x ++)
		{
			i = y * rowstride + x * nchannels;
			aptr = 0;
			for(yo = -1; yo <= 1; yo ++)
				for(xo = -1; xo <= 1; xo ++)
					avals[aptr ++] = pcopy[
						(y + yo) * rowstride + (x + xo) * nchannels];
			pixels[i] = sort_a9(avals);
		}

	g_free(pcopy);
}

guint8 pixbuf_average_r(GdkPixbuf *pixbuf,
	guint32 rx, guint32 ry, guint32 rw, guint32 rh)
{
	guint8 *pixels, *pcopy;
	guint32 width, height, i, rowstride, nchannels;
	guint32 histogram[256];
	gint32 x, y, xo, yo;
	guint32 sum;

	pixels = gdk_pixbuf_get_pixels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	nchannels = gdk_pixbuf_get_n_channels(pixbuf);

	for(i = 0; i < 256; i++)
		histogram[i] = 0;

	pcopy = g_new0(guint8, height * rowstride);
	memcpy(pcopy, pixels, height * rowstride * sizeof(guint8));

	for(y = ry + 1; y < (ry + rh - 1); y ++)
		for(x = rx + 1; x < (rx + rw - 1); x ++)
		{
			i = y * rowstride + x * nchannels;
			sum = 0;
			for(yo = -1; yo <= 1; yo ++)
				for(xo = -1; xo <= 1; xo ++)
					sum += pcopy[
						(y + yo) * rowstride + (x + xo) * nchannels];
			pixels[i] = sum / 9;
			histogram[pixels[i]] ++;
		}

	g_free(pcopy);

	return get_auto_threshold(histogram);
}
