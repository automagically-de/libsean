bin_PROGRAMS = sean-scan

if HAVE_GTK
bin_PROGRAMS += sean-gui
endif

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-I$(top_srcdir)/include \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ \
	@GST_CFLAGS@ \
	@DBG_CFLAGS@

sean_gui_SOURCES = sean-gui.c
sean_gui_DEPENDENCIES = ../src/libsean.la
sean_gui_LDADD = ../src/libsean.la \
	@GTK_LIBS@ @GLIB_LIBS@ @GST_LIBS@ @GDKPIXBUF_LIBS@

sean_scan_SOURCES = sean-scan.c
sean_scan_DEPENDENCIES = ../src/libsean.la
sean_scan_LDADD = ../src/libsean.la \
	@GLIB_LIBS@ @GST_LIBS@ @GDKPIXBUF_LIBS@
