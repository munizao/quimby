#ifndef __CHORDING_H__
#define __CHORDING_H__

typedef enum
{
    NOT_START,
    WORD_START,
    SENTENCE_START
} StartPositionType;

StartPositionType start_position (gchar *text, gint cursor_index);
void load_dictionary (GtkIMQuimbyContext *context);
void add_to_chord (GtkIMQuimbyContext *context, guint keyval);
void clear_chord (GtkIMQuimbyContext *context);
gchar * chord_lookup (GtkIMQuimbyContext *context);

#endif
