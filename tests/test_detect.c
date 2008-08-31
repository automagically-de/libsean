#include <stdio.h>

#include <sean.h>

#define MIN_THR 1
#define MAX_THR 2
#define INC_THR 2

static gfloat pixbuf_get_black_pcnt(GdkPixbuf *pixbuf)
{
	gint32 x, y, width, height, nchannels, rowstride;
	guint8 *pixels;
	guint32 sum;

	sum = 0;

	pixels = gdk_pixbuf_get_pixels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	nchannels = gdk_pixbuf_get_n_channels(pixbuf);

	for(y = 0; y < height; y ++)
		for(x = 0; x < height; x ++)
			if(pixels[y * rowstride + x * nchannels] == 0x00)
				sum ++;

	return (gfloat)sum / (gfloat)(width * height) * 100.0;
}

static guint32 run_test(SeanContext *context, const gchar *title)
{
	GDir *dir;
	GTimeVal tstart, tend;
	guint32 nall, nsucc;
	const gchar *filename;
	gchar *path;
	GdkPixbuf *pixbuf;
	guint32 code[14];
	gfloat black_pcnt_sum;

	nall = nsucc = 0;
	black_pcnt_sum = 0.0;

	g_get_current_time(&tstart);
	dir = g_dir_open("dumps", 0, NULL);
	if(dir == NULL)
		return 0;

	g_print("%-40s: ", title);
	fflush(stdout);

	while((filename = g_dir_read_name(dir)) != NULL)
	{
		nall ++;
		path = g_strdup_printf("dumps/%s", filename);
		pixbuf = gdk_pixbuf_new_from_file(path, NULL);
		if(pixbuf)
		{
			if(sean_scan_pixbuf(context, pixbuf, code))
				nsucc ++;
			black_pcnt_sum += pixbuf_get_black_pcnt(pixbuf);
			gdk_pixbuf_unref(pixbuf);
		}
		g_free(path);
	}

	g_dir_close(dir);
	g_get_current_time(&tend);

	g_print("%5u of %u (%.2f%% black, %.2f fps)\n", nsucc, nall,
		((gdouble)black_pcnt_sum / (gdouble)nall),
		((gdouble)nall / (gdouble)(tend.tv_sec - tstart.tv_sec)));

	return nsucc;
}

int main(int argc, char *argv[])
{
	SeanContext *context;
	guint32 thr, flt, i;
	gchar *title;
	guint8 results[(MAX_THR - MIN_THR) / INC_THR + 1][5];
	FILE *f;

	context = sean_init(0, &argc, &argv);
	g_type_init();

	for(flt = 0; flt < 4; flt ++)
	{
		sean_set_filter(context, flt);
		for(thr = MIN_THR; thr < MAX_THR; thr += INC_THR)
		{
			sean_set_binarize_threshold(context, thr);
			title = g_strdup_printf("(flt: %u) threshold = %u", flt, thr);

			i = (thr - MIN_THR) / INC_THR;
			results[i][0] = thr;
			results[i][1 + flt] = run_test(context, title);

			g_free(title);
		}
	}

	f = fopen("filter.dat", "w");
	if(f)
	{
		for(thr = MIN_THR; thr < MAX_THR; thr += INC_THR)
		{
			i = (thr - MIN_THR) / INC_THR;
			fprintf(f, "%u %u %u %u %u\n", results[i][0], results[i][1],
				results[i][2], results[i][3], results[i][4]);
		}
		fclose(f);
	}

	sean_close(context);

	return EXIT_SUCCESS;
}
