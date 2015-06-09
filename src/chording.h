#ifndef __CHORDING_H__
#define __CHORDING_H__

gboolean word_start_context (gchar *text, gint cursor_index);
void load_dictionary (GtkIMQuimbyContext *context);
void add_to_chord (GtkIMQuimbyContext *context, guint keyval);
void clear_chord (GtkIMQuimbyContext *context);
gchar * chord_lookup (GtkIMQuimbyContext *context);

#endif
