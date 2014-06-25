#include <stdlib.h>
#include <glib.h>

/* Support for chorded input */

static gboolean
word_start_context (gchar *text, gint cursor_index)
{
    gchar *text;
    gint cursor_index;
    gboolean is_start = false;
    gunichar prev = g_utf8_get_char(text+cursor_index);
    gunichar space = 0x20;
    if (prev == space) /* Actual algorithm will be more complicated. */
    {
	is_start = true;
    }
    return is_start;
}

static void
load_dictionary ()
{

}

static void
add_to_chord ()
{
}

static void
empty_chord ()
{

}

static gchar *
chord_lookup ()
{

}

