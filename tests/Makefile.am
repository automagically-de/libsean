noinst_PROGRAMS = test_detect

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-I$(top_srcdir)/include \
	@GLIB_CFLAGS@ \
	@GDKPIXBUF_CFLAGS@ \
	@GST_CFLAGS@ \
	@DBG_CFLAGS@

test_detect_SOURCES = test_detect.c
test_detect_DEPENDENCIES = ../src/libsean.la
test_detect_LDADD = ../src/libsean.la @GLIB_LIBS@ @GST_LIBS@ @GDKPIXBUF_LIBS@
