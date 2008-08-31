lib_LTLIBRARIES = libsean.la

libsean_la_SOURCES = \
	init.c \
	scan.c scan.h \
	pixbuf.c pixbuf.h \
	bar.c bar.h \
	video.c video.h

INCLUDES = -I$(top_builddir)/include

libsean_la_CFLAGS = \
	-Wall -ansi -pedantic \
	-fstrict-aliasing -Wstrict-aliasing \
	-DG_LOG_DOMAIN=\"LibSEAN\" \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@ \
	@GDKPIXBUF_CFLAGS@ \
	@GST_CFLAGS@

libsean_la_LDFLAGS = \
	-version-number "$(subst .,:,@VERSION@)"

libsean_la_LIBADD = \
	@GLIB_LIBS@ \
	@GDKPIXBUF_LIBS@ \
	@GST_LIBS@

