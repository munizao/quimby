#include <stdlib.h>
#include <glib.h>
#include "im-quimby.h"

/* Support for chorded input */

//keys must have characters in unicode order
static gchar* chords[] = {
    "t", "the ",
    "f", "of ",
    "fo", "of ",
    "f", "of ",
    "ft", "of the ",
    "d", "and ",
    "dt", "and the ",
    "a", "a ",
    "n", "in ",
    "in", "in ",
    "nt", "in the ",
    "it", "it ",
    "o", "to ",
    "ot", "to ",
    "s", "is ",
    "is", "is ",
    "aw", "was ",
    "sw", "was ",
    "st", "this ",
    "i", "I ",
    "fr", "for ",
    "/f", "for ",
    "aht", "that ",
    "/t", "that ",
    "w", "with ",
    "tw", "with ", 
    "eh", "he ",
    "b", "be ",
    "be", "be ",
    "beg", "being ",
    "bg", "being ",
    "no", "on ",
    "by", "by ",
    "at", "at ",
    "u" , "you ",
    "k", "can ",
    "kn", "can ",
    "h", "have ",
    "hv", "have ",
    "dh", "had ",
    "ghv", "having ",
    "gh", "having ",
    "ar", "are ",
    "r", "are ",
    "aer", "are ",
    "-", "not ",
    "rw", "were ",
    "bt", "but ",
    "bu", "but ",
    "ty", "they ",
    "fm", "from ",
    "iw", "which ",
    "hs", "she ",
    "es", "she ",
    "or", "or ",
    "an", "an ",
    "ew", "we ",
    "as", "as ",
    "do", "do ",
    "dg", "doing ",
    "dgo", "doing ",
    "di", "did ",
    "bn", "been ",
    "ert", "there ",
    "irt", "their ",
    "lw", "will ",
    "dw", "would ",
    "fi", "if ",
    "sy", "say ",
    "ds", "said ",
    "gs", "saying ",
    "km", "make ",
    "dm", "made ",
    "mg", "making ",
    "kt", "take ", 
    "dhs", "should ",
    "hm", "him ",
    "hr", "her ",
    "c", "see ",
    "cg", "seeing ",
    "cw", "saw ",
    "cn", "seen ",
    "cd", "seed ",
    "/sw", "saw ",
    "/c", "about ",
    "v", "very ",
    "so", "so ",
    "my", "my ",
    "pu", "up ",
    "ou", "out ",
    "al", "all ",
    "~", "about ",
    "yr", "your ",
    "ru", "your ",
    "\'ru", "you\'re ",
    "'ry", "you\'re ",
    "\'u", "you're ",
    "\'y", "you're ",
    "im", "I\'m ",
    "\'i", "I\'m ",
    "\'im", "I\'m ",
    "\'h", "he\'s ",
    "\'s", "she\'s ",
    "\'t", "they\'re ",
    "\'w", "we\'re ",
    "\'k", "can\'t ",
    "tw", "what ",
    "ow", "who ",
    "ms", "some ",
    "lp", "people ",
    "em", "me ",
    "1v", "everyone ",
    "1s", "someone ",
    "-1", "no one ",
    "bc", "because ",
    "bw", "between ",
    "ag", "again ",
    "agt", "against ",
    "j", "just ",
    "fx", "for example ",
    "ax", "ask ",
    "<", "less ",
    ">", "more ",
    "2b", "to be ",
    "2t", "to the ",
    "ez", "easy ",
    "8l", "late ",
    "8r", "rate ",
    "8gr", "rating ",
    "8st", "state",
    "8f", "fate ",
    "8m", "mate ",
    "8d", "date ",
    "4k", "fork ",
    "4c", "foresee ",
    "gmt", "government ",
    "gtv", "government ",
    "fin", "information ",
    "dtv", "development ",
    "rtv", "relevant ",
    "bf", "before ",
    "4b", "before ",
    "hn", "then ",
    "0f", "often",
    "0x", "existence ",
    "0cx", "existence ",
    "0dx", "extend ",
    "mx", "example ",
    "px", "experience ",
    "kl", "like ",
    "fl", "life ",
    "gt", "though ",
    "trg", "through ",
    "ltg", "although ",
    "atg", "although ",
    "4t", "therefore ",
    "dg", "good ",
    "kw", "work ",
    "gw", "working ",
    "gkw", "working ",
    "ay", "any ",
    "ny", "any ",
    "tu", "out ",
    "mn", "many ",
    "du", "under ",
    "bh", "both ",
    "gr", "great ",
    "bs", "somebody ",

    "1", "one ",
    "2", "two ",
    "3", "three ",
    "4", "four ",
    "5", "five ",
    "6", "six ",
    "7", "seven ",
    "8", "eight ",
    "9", "nine ",
    "0", "ten ",
    "01", "ten ",
    "02", "twenty ",
    "03", "thirty ",
    "04", "forty ",
    "05", "fifty ",
    "06", "sixty ",
    "07", "seventy ",
    "08", "eighty ",
    "09", "ninety ",


};

static guint32 keyvals[];

gboolean
word_start_context (gchar *text, gint cursor_index)
{
    // Actual algorithm will still be more complicated.
    if (cursor_index == 0 || g_utf8_get_char(text+cursor_index-1) == 0x20)
    {
	return TRUE;
    }
    return FALSE;
}

void
load_dictionary (GtkIMQuimbyContext *context)
{
    int i;
//    guint32 *chord_keyvals = NULL;
//    guint32 *keyvals = g_malloc0_n(G_N_ELEMENTS(chords), max_chord_len * sizeof(guint32)); 

    context->chord_dict = g_hash_table_new(g_str_hash, g_str_equal);
    for (i = 0; 2*i < G_N_ELEMENTS(chords); i++)
    {
//	chord_keyvals = keyvals + i * MAX_CHORD_KEYS * sizeof(guint32);
	gpointer key = chords[2*i];
	gpointer value = chords[2*i + 1];
	g_hash_table_replace(context->chord_dict, key, value);
    }
}

void
add_to_chord (GtkIMQuimbyContext *context, guint keyval)
{
    //if we want any keys gdk_keyval_to_unicode doesn't catch, we can do so manually.
    guint32 unichar = gdk_keyval_to_unicode(keyval);
    if (unichar == 0x20)
    {
	context->space_in_chord = TRUE;
    }
    else
    {
	if (context->chord_length < MAX_CHORD_CHARS)
	{ 
	    context->chord_chars[context->chord_length] = unichar;
	    context->chord_length++;
	}
    }
}

void
clear_chord (GtkIMQuimbyContext *context)
{
    context->chord_length = 0;
    context->chord_state = FALSE;
    context->space_in_chord = FALSE;
}

gchar *
chord_lookup (GtkIMQuimbyContext *context)
{
    if (!context->space_in_chord)
    {
	return NULL;
    }

    gpointer text_ptr;
    //gconstpointer key;
    int i;
    int j;
    guint32 temp_char;
    gchar buf[50];
    guint pos = 0;
    // insertion sort in place
    for (i = 0; i < context->chord_length; i++)
    {
	j = i;
	while (j > 0 && context->chord_chars[j] < context->chord_chars[j-1])
	{
	    temp_char = context->chord_chars[j];
	    context->chord_chars[j] = context->chord_chars[j-1];
	    context->chord_chars[j-1] = temp_char;
	}
    }
    for (i = 0; i < context->chord_length; i++)
    {
	pos += g_unichar_to_utf8(context->chord_chars[i], buf + pos);
    }

    text_ptr = g_hash_table_lookup(context->chord_dict, buf);
    return text_ptr;
}

