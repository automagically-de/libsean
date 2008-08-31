#ifndef _BAR_H
#define _BAR_H

#include <glib.h>

gboolean bar_read_code(guint8 *pixels, guint32 width, guint32 *code);

#endif /* _BAR_H */
