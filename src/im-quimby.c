/* GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>

#include <gtk/gtkaccelgroup.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkimcontextsimple.h>
#include <gtk/gtkclipboard.h>

#include <gtk/gtkimcontext.h>
#include <gtk/gtkimmodule.h>

#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <string.h>

#include "im-extra-intl.h"
#include "im-quimby.h"
#include "chording.h"

typedef struct _GtkComposeTable GtkComposeTable;

struct _GtkComposeTable 
{
  guint16 *data;
  gint max_seq_len;
  gint n_seqs;
};

/* Once upon a time, I wrote a GTK+ input method with the intent of adding lots of compose 
 * sequences. Since that time, many of these sequences were taken up by stock gtk. 
 * There are still a bunch of sequences I'd like that aren't there, and I want to be able
 * to hack my input method to do other useful things.
 */


/* I should only have to explicitly write my own compose sequences, and not include all of the default ones. For clarity, sequences that appear in the default table should be removed here. */
static guint16 quimby_compose_seqs[] = {
  GDK_Multi_key,	GDK_space,	GDK_Left,	0,	0,	0x2190,	/* LEFTWARDS_ARROW */
  GDK_Multi_key,	GDK_space,	GDK_Up,	0,	0,	0x2191,	/* UPWARDS_ARROW */
  GDK_Multi_key,	GDK_space,	GDK_Right,	0,	0,	0x2192,	/* RIGHTWARDS_ARROW */
  GDK_Multi_key,	GDK_space,	GDK_Down,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
  GDK_Multi_key,	GDK_space,	GDK_KP_Left,	0,	0,	0x2190,	/* LEFTWARDS_ARROW */
  GDK_Multi_key,	GDK_space,	GDK_KP_Up,	0,	0,	0x2191,	/* UPWARDS_ARROW */
  GDK_Multi_key,	GDK_space,	GDK_KP_Right,	0,	0,	0x2192,	/* RIGHTWARDS_ARROW */
  GDK_Multi_key,	GDK_space,	GDK_KP_Down,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
  GDK_Multi_key,	GDK_exclam,	GDK_P,	0,	0,	0x00B6,	/* PILCROW_SIGN */
  GDK_Multi_key,	GDK_exclam,	GDK_S,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_exclam,	GDK_p,	0,	0,	0x00B6,	/* PILCROW_SIGN */
  GDK_Multi_key,	GDK_exclam,	GDK_s,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_quotedbl,	GDK_quotedbl,	0,	0,	0x00A8,	/* DIAERESIS */ 
  GDK_Multi_key,	GDK_quotedbl,	GDK_Left,	0,	0,	0x0308,	/* COMBINING_DIAERESIS */
  GDK_Multi_key,	GDK_quotedbl,	GDK_KP_Left,	0,	0,	0x0308,	/* COMBINING_DIAERESIS */
/*  GDK_Multi_key,	GDK_numbersign,	GDK_numbersign,	0,	0,	0x266F,*/	/* MUSIC_SHARP_SIGN */
  GDK_Multi_key,	GDK_percent,	GDK_0,	0,	0,	0x2030,	/* PER_MILLE_SIGN */
  GDK_Multi_key,	GDK_ampersand,	GDK_A,	0,	0,	0x0391,	/* GREEK_CAPITAL_LETTER_ALPHA */
  GDK_Multi_key,	GDK_ampersand,	GDK_B,	0,	0,	0x0392,	/* GREEK_CAPITAL_LETTER_BETA */
  GDK_Multi_key,	GDK_ampersand,	GDK_C,	0,	0,	0x03A7,	/* GREEK_CAPITAL_LETTER_CHI */
  GDK_Multi_key,	GDK_ampersand,	GDK_D,	0,	0,	0x0394,	/* GREEK_CAPITAL_LETTER_DELTA */
  GDK_Multi_key,	GDK_ampersand,	GDK_E,	0,	0,	0x0395,	/* GREEK_CAPITAL_LETTER_EPSILON */
  GDK_Multi_key,	GDK_ampersand,	GDK_F,	0,	0,	0x03A6,	/* GREEK_CAPITAL_LETTER_PHI */
  GDK_Multi_key,	GDK_ampersand,	GDK_G,	0,	0,	0x0393,	/* GREEK_CAPITAL_LETTER_GAMMA */
  GDK_Multi_key,	GDK_ampersand,	GDK_H,	0,	0,	0x0397,	/* GREEK_CAPITAL_LETTER_ETA */
  GDK_Multi_key,	GDK_ampersand,	GDK_I,	0,	0,	0x0399,	/* GREEK_CAPITAL_LETTER_IOTA */
  GDK_Multi_key,	GDK_ampersand,	GDK_J,	0,	0,	0x03D1,	/* GREEK_THETA_SYMBOL */
  GDK_Multi_key,	GDK_ampersand,	GDK_K,	0,	0,	0x039A,	/* GREEK_CAPITAL_LETTER_KAPPA */
  GDK_Multi_key,	GDK_ampersand,	GDK_L,	0,	0,	0x039B,	/* GREEK_CAPITAL_LETTER_LAMBDA */
  GDK_Multi_key,	GDK_ampersand,	GDK_M,	0,	0,	0x039C,	/* GREEK_CAPITAL_LETTER_MU */
  GDK_Multi_key,	GDK_ampersand,	GDK_N,	0,	0,	0x039D,	/* GREEK_CAPITAL_LETTER_NU */
  GDK_Multi_key,	GDK_ampersand,	GDK_O,	0,	0,	0x039F,	/* GREEK_CAPITAL_LETTER_OMICRON */
  GDK_Multi_key,	GDK_ampersand,	GDK_P,	0,	0,	0x03A0,	/* GREEK_CAPITAL_LETTER_PI */
  GDK_Multi_key,	GDK_ampersand,	GDK_Q,	0,	0,	0x0398,	/* GREEK_CAPITAL_LETTER_THETA */
  GDK_Multi_key,	GDK_ampersand,	GDK_R,	0,	0,	0x03A1,	/* GREEK_CAPITAL_LETTER_RHO */
  GDK_Multi_key,	GDK_ampersand,	GDK_S,	0,	0,	0x03A3,	/* GREEK_CAPITAL_LETTER_SIGMA */
  GDK_Multi_key,	GDK_ampersand,	GDK_T,	0,	0,	0x03A4,	/* GREEK_CAPITAL_LETTER_TAU */
  GDK_Multi_key,	GDK_ampersand,	GDK_U,	0,	0,	0x03A5,	/* GREEK_CAPITAL_LETTER_UPSILON */
  GDK_Multi_key,	GDK_ampersand,	GDK_V,	0,	0,	0x03C2,	/* GREEK_SMALL_LETTER_FINAL_SIGMA */
  GDK_Multi_key,	GDK_ampersand,	GDK_W,	0,	0,	0x03A9,	/* GREEK_CAPITAL_LETTER_OMEGA */
  GDK_Multi_key,	GDK_ampersand,	GDK_X,	0,	0,	0x039E,	/* GREEK_CAPITAL_LETTER_XI */
  GDK_Multi_key,	GDK_ampersand,	GDK_Y,	0,	0,	0x03A8,	/* GREEK_CAPITAL_LETTER_PSI */
  GDK_Multi_key,	GDK_ampersand,	GDK_Z,	0,	0,	0x0396,	/* GREEK_CAPITAL_LETTER_ZETA */
  GDK_Multi_key,	GDK_ampersand,	GDK_a,	0,	0,	0x03B1,	/* GREEK_SMALL_LETTER_ALPHA */
  GDK_Multi_key,	GDK_ampersand,	GDK_b,	0,	0,	0x03B2,	/* GREEK_SMALL_LETTER_BETA */
  GDK_Multi_key,	GDK_ampersand,	GDK_c,	0,	0,	0x03C7,	/* GREEK_SMALL_LETTER_CHI */
  GDK_Multi_key,	GDK_ampersand,	GDK_d,	0,	0,	0x03B4,	/* GREEK_SMALL_LETTER_DELTA */
  GDK_Multi_key,	GDK_ampersand,	GDK_e,	0,	0,	0x03B5,	/* GREEK_SMALL_LETTER_EPSILON */
  GDK_Multi_key,	GDK_ampersand,	GDK_f,	0,	0,	0x03C6,	/* GREEK_SMALL_LETTER_PHI */
  GDK_Multi_key,	GDK_ampersand,	GDK_g,	0,	0,	0x03B3,	/* GREEK_SMALL_LETTER_GAMMA */
  GDK_Multi_key,	GDK_ampersand,	GDK_h,	0,	0,	0x03B7,	/* GREEK_SMALL_LETTER_ETA */
  GDK_Multi_key,	GDK_ampersand,	GDK_i,	0,	0,	0x03B9,	/* GREEK_SMALL_LETTER_IOTA */
  GDK_Multi_key,	GDK_ampersand,	GDK_j,	0,	0,	0x03D5,	/* GREEK_PHI_SYMBOL */
  GDK_Multi_key,	GDK_ampersand,	GDK_k,	0,	0,	0x03BA,	/* GREEK_SMALL_LETTER_KAPPA */
  GDK_Multi_key,	GDK_ampersand,	GDK_l,	0,	0,	0x03BB,	/* GREEK_SMALL_LETTER_LAMBDA */
  GDK_Multi_key,	GDK_ampersand,	GDK_m,	0,	0,	0x03BC,	/* GREEK_SMALL_LETTER_MU */
  GDK_Multi_key,	GDK_ampersand,	GDK_n,	0,	0,	0x03BD,	/* GREEK_SMALL_LETTER_NU */
  GDK_Multi_key,	GDK_ampersand,	GDK_o,	0,	0,	0x03BF,	/* GREEK_SMALL_LETTER_OMICRON */
  GDK_Multi_key,	GDK_ampersand,	GDK_p,	0,	0,	0x03C0,	/* GREEK_SMALL_LETTER_PI */
  GDK_Multi_key,	GDK_ampersand,	GDK_q,	0,	0,	0x03B8,	/* GREEK_SMALL_LETTER_THETA */
  GDK_Multi_key,	GDK_ampersand,	GDK_r,	0,	0,	0x03C1,	/* GREEK_SMALL_LETTER_RHO */
  GDK_Multi_key,	GDK_ampersand,	GDK_s,	0,	0,	0x03C3,	/* GREEK_SMALL_LETTER_SIGMA */
  GDK_Multi_key,	GDK_ampersand,	GDK_t,	0,	0,	0x03C4,	/* GREEK_SMALL_LETTER_TAU */
  GDK_Multi_key,	GDK_ampersand,	GDK_u,	0,	0,	0x03C5,	/* GREEK_SMALL_LETTER_UPSILON */
  GDK_Multi_key,	GDK_ampersand,	GDK_v,	0,	0,	0x03D6,	/* GREEK_PI_SYMBOL */
  GDK_Multi_key,	GDK_ampersand,	GDK_w,	0,	0,	0x03C9,	/* GREEK_SMALL_LETTER_OMEGA */
  GDK_Multi_key,	GDK_ampersand,	GDK_x,	0,	0,	0x03BE,	/* GREEK_SMALL_LETTER_XI */
  GDK_Multi_key,	GDK_ampersand,	GDK_y,	0,	0,	0x03C8,	/* GREEK_SMALL_LETTER_PSI */
  GDK_Multi_key,	GDK_ampersand,	GDK_z,	0,	0,	0x03B6,	/* GREEK_SMALL_LETTER_ZETA */
  GDK_Multi_key,	GDK_apostrophe,	GDK_Left,	0,	0,	0x0301,	/* COMBINING_ACUTE_ACCENT */
  GDK_Multi_key,	GDK_apostrophe,	GDK_KP_Left,	0,	0,	0x0301,	/* COMBINING_ACUTE_ACCENT */
  GDK_Multi_key,	GDK_parenleft,	GDK_parenright,	0,	0,	0x25CB,	/* WHITE_CIRCLE */
  GDK_Multi_key,	GDK_parenleft,	GDK_asterisk,	0,	0,	0x262A,	/* STAR_AND_CRESCENT */
/* TODO: check reverse breve seqs */
  GDK_Multi_key,	GDK_parenleft,	GDK_E,	0,	0,	0x0114,	/* LATIN_CAPITAL_LETTER_E_WITH_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_I,	0,	0,	0x012C,	/* LATIN_CAPITAL_LETTER_I_WITH_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_O,	0,	0,	0x014E,	/* LATIN_CAPITAL_LETTER_O_WITH_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_U,	0,	0,	0x016C,	/* LATIN_CAPITAL_LETTER_U_WITH_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_e,	0,	0,	0x0115,	/* LATIN_SMALL_LETTER_E_WITH_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_i,	0,	0,	0x012D,	/* LATIN_SMALL_LETTER_I_WITH_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_o,	0,	0,	0x014F,	/* LATIN_SMALL_LETTER_O_WITH_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_u,	0,	0,	0x016D,	/* LATIN_SMALL_LETTER_U_WITH_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_Left,	0,	0,	0x0306,	/* COMBINING_BREVE */
  GDK_Multi_key,	GDK_parenleft,	GDK_KP_Left,	0,	0,	0x0306,	/* COMBINING_BREVE */
  GDK_Multi_key,	GDK_asterisk,	GDK_parenleft,	0,	0,	0x262A,	/* STAR_AND_CRESCENT */
  GDK_Multi_key,	GDK_asterisk,	GDK_asterisk,	0,	0,	0x2218,	/* RING_OPERATOR */
  GDK_Multi_key,	GDK_asterisk,	GDK_Left,	0,	0,	0x030A,	/* COMBINING_RING_ABOVE */
  GDK_Multi_key,	GDK_asterisk,	GDK_KP_Left,	0,	0,	0x030A,	/* COMBINING_RING_ABOVE */
  GDK_Multi_key,	GDK_plus,	GDK_minus,	0,	0,	0x00B1,	/* PLUSxMINUS_SIGN */
  GDK_Multi_key,	GDK_plus,	GDK_0,	0,	0,	0x2295,	/* CIRCLED_PLUS */
  GDK_Multi_key,	GDK_plus,	GDK_O,	0,	0,	0x2295,	/* CIRCLED_PLUS */
  GDK_Multi_key,	GDK_plus,	GDK_o,	0,	0,	0x2295,	/* CIRCLED_PLUS */
  GDK_Multi_key,	GDK_comma,	GDK_Left,	0,	0,	0x0328,	/* COMBINING_OGONEK */
  GDK_Multi_key,	GDK_comma,	GDK_KP_Left,	0,	0,	0x0328,	/* COMBINING_OGONEK */
  GDK_Multi_key,	GDK_comma,	GDK_O,	0,	0,	0x01EA,	/* LATIN_CAPITAL_LETTER_O_WITH_OGONEK */
  GDK_Multi_key,	GDK_comma,	GDK_o,	0,	0,	0x01EB,	/* LATIN_SMALL_LETTER_O_WITH_OGONEK */
  GDK_Multi_key,	GDK_minus,	GDK_space,	0,	0,	0x007E,	/* TILDE */
  GDK_Multi_key,	GDK_minus,	GDK_parenleft,	0,	0,	0x007B,	/* LEFT_CURLY_BRACKET */
  GDK_Multi_key,	GDK_minus,	GDK_parenright,	0,	0,	0x007D,	/* RIGHT_CURLY_BRACKET */
  GDK_Multi_key,	GDK_minus,	GDK_plus,	0,	0,	0x00B1,	/* PLUSxMINUS_SIGN */
  GDK_Multi_key,	GDK_minus,	GDK_comma,	0,	0,	0x00AC,	/* NOT_SIGN */
  GDK_Multi_key,	GDK_minus,	GDK_minus,	0,	0,	0x00AD,	/* SOFT_HYPHEN */
  GDK_Multi_key,	GDK_minus,	GDK_equal,	0,	0,	0x2261,	/* IDENTICAL_TO */
  GDK_Multi_key,	GDK_minus,	GDK_A,	0,	0,	0x0100,	/* LATIN_CAPITAL_LETTER_A_WITH_MACRON */
  GDK_Multi_key,	GDK_minus,	GDK_D,	0,	0,	0x0110,	/* LATIN_CAPITAL_LETTER_D_WITH_STROKE */
  GDK_Multi_key,	GDK_minus,	GDK_E,	0,	0,	0x0112,	/* LATIN_CAPITAL_LETTER_E_WITH_MACRON */
  GDK_Multi_key,	GDK_minus,	GDK_H,	0,	0,	0x0126,	/* LATIN_CAPITAL_LETTER_H_WITH_STROKE */
  GDK_Multi_key,	GDK_minus,	GDK_I,	0,	0,	0x012A,	/* LATIN_CAPITAL_LETTER_I_WITH_MACRON */
  GDK_Multi_key,	GDK_minus,	GDK_L,	0,	0,	0x00A3,	/* POUND_SIGN */
  GDK_Multi_key,	GDK_minus,	GDK_M,	0,	0,	0x2014,	/* EM_DASH */
  GDK_Multi_key,	GDK_minus,	GDK_N,	0,	0,	0x00D1,	/* LATIN_CAPITAL_LETTER_N_WITH_TILDE */
  GDK_Multi_key,	GDK_minus,	GDK_O,	0,	0,	0x014C,	/* LATIN_CAPITAL_LETTER_O_WITH_MACRON */
  GDK_Multi_key,	GDK_minus,	GDK_U,	0,	0,	0x016A,	/* LATIN_CAPITAL_LETTER_U_WITH_MACRON */
  GDK_Multi_key,	GDK_minus,	GDK_Y,	0,	0,	0x00A5,	/* YEN_SIGN */
  GDK_Multi_key,	GDK_minus,	GDK_bracketleft,	0,	0,	0x2208,	/* ELEMENT_OF */
  GDK_Multi_key,	GDK_minus,	GDK_bracketright,	0,	0,	0x220B,	/* CONTAINS_AS_MEMBER */
  GDK_Multi_key,	GDK_minus,	GDK_asciicircum,	0,	0,	0x00AF,	/* MACRON */
  GDK_Multi_key,	GDK_minus,	GDK_d,	0,	0,	0x0111,	/* LATIN_SMALL_LETTER_D_WITH_STROKE */
  GDK_Multi_key,	GDK_minus,	GDK_h,	0,	0,	0x0127,	/* LATIN_SMALL_LETTER_H_WITH_STROKE */
  GDK_Multi_key,	GDK_minus,	GDK_l,	0,	0,	0x00A3,	/* POUND_SIGN */
  GDK_Multi_key,	GDK_minus,	GDK_m,	0,	0,	0x2014,	/* EM_DASH */
  GDK_Multi_key,	GDK_minus,	GDK_s,	0,	0,	0x017F,	/* LATIN_SMALL_LETTER_LONG_S */
  GDK_Multi_key,	GDK_minus,	GDK_y,	0,	0,	0x00A5,	/* YEN_SIGN */
  GDK_Multi_key,	GDK_minus,	GDK_bar,	0,	0,	0x2020,	/* DAGGER */
  GDK_Multi_key,	GDK_minus,	GDK_Left,	0,	0,	0x0304,	/* COMBINING_MACRON */
  GDK_Multi_key,	GDK_minus,	GDK_KP_Left,	0,	0,	0x0304,	/* COMBINING_MACRON */
  /* GDK_Multi_key,	GDK_period,	GDK_period,	0,	0,	0x00B7, */	/* MIDDLE_DOT */ /* conflicts with ellipsis. I use this more, but I made Altgr+. = this */
  GDK_Multi_key,	GDK_period,	GDK_colon,	0,	0,	0x2234,	/* THEREFORE */
  GDK_Multi_key,	GDK_period,	GDK_L,	0,	0,	0x013F,	/* LATIN_CAPITAL_LETTER_L_WITH_MIDDLE_DOT */
  GDK_Multi_key,	GDK_period,	GDK_asciicircum,	0,	0,	0x02D9,	/* DOT_ABOVE */
  GDK_Multi_key,	GDK_period,	GDK_l,	0,	0,	0x0140,	/* LATIN_SMALL_LETTER_L_WITH_MIDDLE_DOT */
  GDK_Multi_key,	GDK_period,	GDK_Left,	0,	0,	0x0307,	/* COMBINING_DOT_ABOVE */
  GDK_Multi_key,	GDK_period,	GDK_KP_Left,	0,	0,	0x0307,	/* COMBINING_DOT_ABOVE */
  GDK_Multi_key,	GDK_slash,	GDK_0,	0,	0,	0x2205,	/* EMPTY_SET */
  GDK_Multi_key,	GDK_slash,	GDK_equal,	0,	0,	0x2260,	/* NOT_EQUAL_TO */
  GDK_Multi_key,	GDK_slash,	GDK_D,	0,	0,	0x00D0,	/* LATIN_CAPITAL_LETTER_ETH */
  GDK_Multi_key,	GDK_slash,	GDK_H,	0,	0,	0x210F,	/* PLANCK_CONSTANT_OVER_TWO_PI */
  GDK_Multi_key,	GDK_slash,	GDK_L,	0,	0,	0x0141,	/* LATIN_CAPITAL_LETTER_L_WITH_STROKE */
  GDK_Multi_key,	GDK_slash,	GDK_O,	0,	0,	0x00D8,	/* LATIN_CAPITAL_LETTER_O_WITH_STROKE */
  GDK_Multi_key,	GDK_slash,	GDK_R,	0,	0,	0x211E,	/* PRESCRIPTION_TAKE */
  GDK_Multi_key,	GDK_slash,	GDK_T,	0,	0,	0x0166,	/* LATIN_CAPITAL_LETTER_T_WITH_STROKE */
  GDK_Multi_key,	GDK_slash,	GDK_U,	0,	0,	0x00B5,	/* MICRO_SIGN */
  GDK_Multi_key,	GDK_slash,	GDK_backslash,	0,	0,	0x2227,	/* LOGICAL_AND */
  GDK_Multi_key,	GDK_slash,	GDK_asciicircum,	0,	0,	0x007C,	/* VERTICAL_LINE */
  GDK_Multi_key,	GDK_slash,	GDK_underscore,	0,	0,	0x2220,	/* ANGLE */
  GDK_Multi_key,	GDK_slash,	GDK_d,	0,	0,	0x00F0,	/* LATIN_SMALL_LETTER_ETH */
  GDK_Multi_key,	GDK_slash,	GDK_h,	0,	0,	0x210F,	/* PLANCK_CONSTANT_OVER_TWO_PI */
  GDK_Multi_key,	GDK_slash,	GDK_r,	0,	0,	0x211E,	/* PRESCRIPTION_TAKE */
  GDK_Multi_key,	GDK_slash,	GDK_v,	0,	0,	0x2123,	/* VERSICLE */
  GDK_Multi_key,	GDK_slash,	GDK_bar,	0,	0,	0x2224,	/* DOES_NOT_DIVIDE */
  GDK_Multi_key,	GDK_slash,	GDK_Left,	0,	0,	0x219A,	/* LEFTWARDS_ARROW_WITH_STROKE */
  GDK_Multi_key,	GDK_slash,	GDK_Right,	0,	0,	0x219B,	/* RIGHTWARDS_ARROW_WITH_STROKE */
  GDK_Multi_key,	GDK_slash,	GDK_KP_Left,	0,	0,	0x219A,	/* LEFTWARDS_ARROW_WITH_STROKE */
  GDK_Multi_key,	GDK_slash,	GDK_KP_Right,	0,	0,	0x219B,	/* RIGHTWARDS_ARROW_WITH_STROKE */
  GDK_Multi_key,	GDK_0,	GDK_parenleft,	0,	0,	0x221D,	/* PROPORTIONAL_TO */
  GDK_Multi_key,	GDK_0,	GDK_plus,	0,	0,	0x2295,	/* CIRCLED_PLUS */
  GDK_Multi_key,	GDK_0,	GDK_slash,	0,	0,	0x2205,	/* EMPTY_SET */
  GDK_Multi_key,	GDK_0,	GDK_0,	0,	0,	0x221E,	/* INFINITY */
  GDK_Multi_key,	GDK_0,	GDK_C,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_0,	GDK_P,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_0,	GDK_R,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_0,	GDK_X,	0,	0,	0x2297,	/* CIRCLED_TIMES */
  GDK_Multi_key,	GDK_0,	GDK_asciicircum,	0,	0,	0x00B0,	/* DEGREE_SIGN */
  GDK_Multi_key,	GDK_0,	GDK_underscore,	0,	0,	0x2080,	/* SUBSCRIPT_ZERO */
  GDK_Multi_key,	GDK_0,	GDK_c,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_0,	GDK_p,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_0,	GDK_r,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_0,	GDK_x,	0,	0,	0x2297,	/* CIRCLED_TIMES */
  GDK_Multi_key,	GDK_0,	GDK_Right,	0,	0,	0x21F4,	/* RIGHT_ARROW_WITH_SMALL_CIRCLE */
  GDK_Multi_key,	GDK_0,	GDK_KP_Right,	0,	0,	0x21F4,	/* RIGHT_ARROW_WITH_SMALL_CIRCLE */
  GDK_Multi_key,	GDK_1,	GDK_slash,	0,	0,	0x215F,	/* FRACTION_NUMERATOR_ONE */
  GDK_Multi_key,	GDK_1,	GDK_asciicircum,	0,	0,	0x00B9,	/* SUPERSCRIPT_ONE */
  GDK_Multi_key,	GDK_1,	GDK_underscore,	0,	0,	0x2081,	/* SUBSCRIPT_ONE */
  GDK_Multi_key,	GDK_2,	GDK_asciicircum,	0,	0,	0x00B2,	/* SUPERSCRIPT_TWO */
  GDK_Multi_key,	GDK_2,	GDK_underscore,	0,	0,	0x2082,	/* SUBSCRIPT_TWO */
  GDK_Multi_key,	GDK_2,	GDK_Left,	0,	0,	0x21C7,	/* LEFTWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_2,	GDK_Up,	0,	0,	0x21C8,	/* UPWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_2,	GDK_Right,	0,	0,	0x21C9,	/* RIGHTWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_2,	GDK_Down,	0,	0,	0x21CA,	/* DOWNWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_2,	GDK_KP_Left,	0,	0,	0x21C7,	/* LEFTWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_2,	GDK_KP_Up,	0,	0,	0x21C8,	/* UPWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_2,	GDK_KP_Right,	0,	0,	0x21C9,	/* RIGHTWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_2,	GDK_KP_Down,	0,	0,	0x21CA,	/* DOWNWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_3,	GDK_S,	0,	0,	0x00B3,	/* SUPERSCRIPT_THREE */
  GDK_Multi_key,	GDK_3,	GDK_asciicircum,	0,	0,	0x00B3,	/* SUPERSCRIPT_THREE */
  GDK_Multi_key,	GDK_3,	GDK_underscore,	0,	0,	0x2083,	/* SUBSCRIPT_THREE */
  GDK_Multi_key,	GDK_3,	GDK_s,	0,	0,	0x00B3,	/* SUPERSCRIPT_THREE */
  GDK_Multi_key,	GDK_3,	GDK_Right,	0,	0,	0x21F6,	/* THREE_RIGHTWARDS_ARROWS */
  GDK_Multi_key,	GDK_3,	GDK_KP_Right,	0,	0,	0x21F6,	/* THREE_RIGHTWARDS_ARROWS */
  GDK_Multi_key,	GDK_4,	GDK_S,	0,	0,	0x2074,	/* SUPERSCRIPT_FOUR */
  GDK_Multi_key,	GDK_4,	GDK_asciicircum,	0,	0,	0x2074,	/* SUPERSCRIPT_FOUR */
  GDK_Multi_key,	GDK_4,	GDK_underscore,	0,	0,	0x2084,	/* SUBSCRIPT_FOUR */
  GDK_Multi_key,	GDK_4,	GDK_s,	0,	0,	0x2074,	/* SUPERSCRIPT_FOUR */
  GDK_Multi_key,	GDK_5,	GDK_S,	0,	0,	0x2075,	/* SUPERSCRIPT_FIVE */
  GDK_Multi_key,	GDK_5,	GDK_asciicircum,	0,	0,	0x2075,	/* SUPERSCRIPT_FIVE */
  GDK_Multi_key,	GDK_5,	GDK_underscore,	0,	0,	0x2085,	/* SUBSCRIPT_FIVE */
  GDK_Multi_key,	GDK_5,	GDK_s,	0,	0,	0x2076,	/* SUPERSCRIPT_FIVE */
  GDK_Multi_key,	GDK_6,	GDK_S,	0,	0,	0x2076,	/* SUPERSCRIPT_SIX */
  GDK_Multi_key,	GDK_6,	GDK_asciicircum,	0,	0,	0x2076,	/* SUPERSCRIPT_SIX */
  GDK_Multi_key,	GDK_6,	GDK_underscore,	0,	0,	0x2086,	/* SUBSCRIPT_SIX */
  GDK_Multi_key,	GDK_6,	GDK_s,	0,	0,	0x2076,	/* SUPERSCRIPT_SIX */
  GDK_Multi_key,	GDK_7,	GDK_S,	0,	0,	0x2077,	/* SUPERSCRIPT_SEVEN */
  GDK_Multi_key,	GDK_7,	GDK_asciicircum,	0,	0,	0x2077,	/* SUPERSCRIPT_SEVEN */
  GDK_Multi_key,	GDK_7,	GDK_underscore,	0,	0,	0x2087,	/* SUBSCRIPT_SEVEN */
  GDK_Multi_key,	GDK_7,	GDK_s,	0,	0,	0x2077,	/* SUPERSCRIPT_SEVEN */
  GDK_Multi_key,	GDK_8,	GDK_8,	0,	0,	0x221E,	/* INFINITY */
  GDK_Multi_key,	GDK_8,	GDK_S,	0,	0,	0x2078,	/* SUPERSCRIPT_EIGHT */
  GDK_Multi_key,	GDK_8,	GDK_asciicircum,	0,	0,	0x2078,	/* SUPERSCRIPT_EIGHT */
  GDK_Multi_key,	GDK_8,	GDK_underscore,	0,	0,	0x2088,	/* SUBSCRIPT_EIGHT */
  GDK_Multi_key,	GDK_8,	GDK_s,	0,	0,	0x2078,	/* SUPERSCRIPT_EIGHT */
  GDK_Multi_key,	GDK_9,	GDK_S,	0,	0,	0x2079,	/* SUPERSCRIPT_NINE */
  GDK_Multi_key,	GDK_9,	GDK_asciicircum,	0,	0,	0x2079,	/* SUPERSCRIPT_NINE */
  GDK_Multi_key,	GDK_9,	GDK_underscore,	0,	0,	0x2089,	/* SUBSCRIPT_NINE */
  GDK_Multi_key,	GDK_9,	GDK_s,	0,	0,	0x2079,	/* SUPERSCRIPT_NINE */
  GDK_Multi_key,	GDK_colon,	GDK_period,	0,	0,	0x2234,	/* THEREFORE */
  GDK_Multi_key,	GDK_colon,	GDK_colon,	0,	0,	0x2237,	/* PROPORTION */
  GDK_Multi_key,	GDK_colon,	GDK_O,	0,	0,	0x0150,	/* LATIN_CAPITAL_LETTER_O_WITH_DOUBLE_ACUTE */
  GDK_Multi_key,	GDK_colon,	GDK_U,	0,	0,	0x0170,	/* LATIN_CAPITAL_LETTER_U_WITH_DOUBLE_ACUTE */
  GDK_Multi_key,	GDK_colon,	GDK_o,	0,	0,	0x0151,	/* LATIN_SMALL_LETTER_O_WITH_DOUBLE_ACUTE */
  GDK_Multi_key,	GDK_colon,	GDK_u,	0,	0,	0x0171,	/* LATIN_SMALL_LETTER_U_WITH_DOUBLE_ACUTE */
  GDK_Multi_key,	GDK_colon,	GDK_Left,	0,	0,	0x030B,	/* COMBINING_DOUBLE_ACUTE_ACCENT */
  GDK_Multi_key,	GDK_colon,	GDK_KP_Left,	0,	0,	0x030B,	/* COMBINING_DOUBLE_ACUTE_ACCENT */
  GDK_Multi_key,	GDK_less,	GDK_minus,	0,	0,	0x2190,	/* LEFTWARDS_ARROW */
  GDK_Multi_key,	GDK_less,	GDK_greater,	0,	0,	0x25C7,	/* WHITE_DIAMOND */
  GDK_Multi_key,	GDK_less,	GDK_Left,	0,	0,	0x030C,	/* COMBINING_CARON */
  GDK_Multi_key,	GDK_less,	GDK_KP_Left,	0,	0,	0x030C,	/* COMBINING_CARON */
  GDK_Multi_key,	GDK_less,	GDK_bar,	0,	0,	0x2206,	/* INCREMENT */
  GDK_Multi_key,	GDK_equal,	GDK_minus,	0,	0,	0x2261,	/* IDENTICAL_TO */
  GDK_Multi_key,	GDK_equal,	GDK_less,	0,	0,	0x2264,	/* LESS-THAN_OR_EQUAL_TO */
  GDK_Multi_key,	GDK_equal,	GDK_greater,	0,	0,	0x2265,	/* GREATER-THAN_OR_EQUAL_TO */
  GDK_Multi_key,	GDK_equal,	GDK_bar,	0,	0,	0x2021,	/* DOUBLE_DAGGER */
  GDK_Multi_key,	GDK_equal,	GDK_Left,	0,	0,	0x21D0,	/* LEFTWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_equal,	GDK_Up,	0,	0,	0x21D1,	/* UPWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_equal,	GDK_Right,	0,	0,	0x21D2,	/* RIGHTWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_equal,	GDK_Down,	0,	0,	0x21D3,	/* DOWNWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_equal,	GDK_KP_Left,	0,	0,	0x21D0,	/* LEFTWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_equal,	GDK_KP_Up,	0,	0,	0x21D1,	/* UPWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_equal,	GDK_KP_Right,	0,	0,	0x21D2,	/* RIGHTWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_equal,	GDK_KP_Down,	0,	0,	0x21D3,	/* DOWNWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_greater,	GDK_equal,	0,	0,	0x2265,	/* GREATER-THAN_OR_EQUAL_TO */
  GDK_Multi_key,	GDK_greater,	GDK_Left,	0,	0,	0x0302,	/* COMBINING_CIRCUMFLEX_ACCENT */
  GDK_Multi_key,	GDK_greater,	GDK_KP_Left,	0,	0,	0x0302,	/* COMBINING_CIRCUMFLEX_ACCENT */
  GDK_Multi_key,	GDK_C,	GDK_equal,	0,	0,	0x20AC,	/* EURO_SIGN */
  GDK_Multi_key,	GDK_C,	GDK_greater,	0,	0,	0x0108,	/* LATIN_CAPITAL_LETTER_C_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_C,	GDK_L,	0,	0,	0x2104,	/* CENTRE_LINE_SYMBOL */
  GDK_Multi_key,	GDK_C,	GDK_O,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_C,	GDK_R,	0,	0,	0x20A2,	/* CRUZEIRO_SIGN */
  GDK_Multi_key,	GDK_C,	GDK_asciicircum,	0,	0,	0x0108,	/* LATIN_CAPITAL_LETTER_C_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_C,	GDK_o,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_C,	GDK_r,	0,	0,	0x20A2,	/* CRUZEIRO_SIGN */
  GDK_Multi_key,	GDK_C,	GDK_bar,	0,	0,	0x00A2,	/* CENT_SIGN */
  GDK_Multi_key,	GDK_D,	GDK_minus,	0,	0,	0x0110,	/* LATIN_CAPITAL_LETTER_D_WITH_STROKE */
  GDK_Multi_key,	GDK_D,	GDK_period,	0,	0,	0x1E0A,	/* LATIN_CAPITAL_LETTER_D_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_D,	GDK_slash,	0,	0,	0x00D0,	/* LATIN_CAPITAL_LETTER_ETH */
  GDK_Multi_key,	GDK_E,	GDK_equal,	0,	0,	0x20AC,	/* EURO_SIGN */
  GDK_Multi_key,	GDK_E,	GDK_greater,	0,	0,	0x00CA,	/* LATIN_CAPITAL_LETTER_E_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_E,	GDK_E,          0,	0,	0x2203,	/* THERE_EXISTS */
  GDK_Multi_key,	GDK_E,	GDK_breve,	0,	0,	0x0114,	/* LATIN_CAPITAL_LETTER_E_WITH_BREVE */
  GDK_Multi_key,	GDK_F,	GDK_period,	0,	0,	0x1E1E,	/* LATIN_CAPITAL_LETTER_F_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_G,	GDK_parenleft,	0,	0,	0x011E,	/* LATIN_CAPITAL_LETTER_G_WITH_BREVE */
  GDK_Multi_key,	GDK_G,	GDK_comma,	0,	0,	0x0122,	/* LATIN_CAPITAL_LETTER_G_WITH_CEDILLA */
  GDK_Multi_key,	GDK_G,	GDK_period,	0,	0,	0x0120,	/* LATIN_CAPITAL_LETTER_G_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_G,	GDK_greater,	0,	0,	0x011C,	/* LATIN_CAPITAL_LETTER_G_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_G,	GDK_asciicircum,	0,	0,	0x011C,	/* LATIN_CAPITAL_LETTER_G_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_G,	GDK_cedilla,	0,	0,	0x0122,	/* LATIN_CAPITAL_LETTER_G_WITH_CEDILLA */
  GDK_Multi_key,	GDK_G,	GDK_breve,	0,	0,	0x011E,	/* LATIN_CAPITAL_LETTER_G_WITH_BREVE */
  GDK_Multi_key,	GDK_H,	GDK_minus,	0,	0,	0x0126,	/* LATIN_CAPITAL_LETTER_H_WITH_STROKE */
  GDK_Multi_key,	GDK_H,	GDK_slash,	0,	0,	0x210F,	/* PLANCK_CONSTANT_OVER_TWO_PI */
  GDK_Multi_key,	GDK_H,	GDK_greater,	0,	0,	0x0124,	/* LATIN_CAPITAL_LETTER_H_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_H,	GDK_asciicircum,	0,	0,	0x0124,	/* LATIN_CAPITAL_LETTER_H_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_I,	GDK_quotedbl,	0,	0,	0x00CF,	/* LATIN_CAPITAL_LETTER_I_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_I,	GDK_apostrophe,	0,	0,	0x00CD,	/* LATIN_CAPITAL_LETTER_I_WITH_ACUTE */
  GDK_Multi_key,	GDK_I,	GDK_parenleft,	0,	0,	0x012C,	/* LATIN_CAPITAL_LETTER_I_WITH_BREVE */
  GDK_Multi_key,	GDK_I,	GDK_comma,	0,	0,	0x012E,	/* LATIN_CAPITAL_LETTER_I_WITH_OGONEK */
  GDK_Multi_key,	GDK_I,	GDK_minus,	0,	0,	0x012A,	/* LATIN_CAPITAL_LETTER_I_WITH_MACRON */
  GDK_Multi_key,	GDK_I,	GDK_period,	0,	0,	0x0130,	/* LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_I,	GDK_greater,	0,	0,	0x00CE,	/* LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_I,	GDK_asciicircum,	0,	0,	0x00CE,	/* LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_I,	GDK_underscore,	0,	0,	0x012A,	/* LATIN_CAPITAL_LETTER_I_WITH_MACRON */
  GDK_Multi_key,	GDK_I,	GDK_grave,	0,	0,	0x00CC,	/* LATIN_CAPITAL_LETTER_I_WITH_GRAVE */
  GDK_Multi_key,	GDK_I,	GDK_J,	0,	0,	0x0132,	/* LATIN_CAPITAL_LIGATURE_IJ */
  GDK_Multi_key,	GDK_I,	GDK_asciitilde,	0,	0,	0x0128,	/* LATIN_CAPITAL_LETTER_I_WITH_TILDE */
  GDK_Multi_key,	GDK_I,	GDK_diaeresis,	0,	0,	0x00CF,	/* LATIN_CAPITAL_LETTER_I_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_I,	GDK_acute,	0,	0,	0x00CD,	/* LATIN_CAPITAL_LETTER_I_WITH_ACUTE */
  GDK_Multi_key,	GDK_I,	GDK_breve,	0,	0,	0x012C,	/* LATIN_CAPITAL_LETTER_I_WITH_BREVE */
  GDK_Multi_key,	GDK_J,	GDK_greater,	0,	0,	0x0134,	/* LATIN_CAPITAL_LETTER_J_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_J,	GDK_asciicircum,	0,	0,	0x0134,	/* LATIN_CAPITAL_LETTER_J_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_K,	GDK_comma,	0,	0,	0x0136,	/* LATIN_CAPITAL_LETTER_K_WITH_CEDILLA */
  GDK_Multi_key,	GDK_K,	GDK_cedilla,	0,	0,	0x0136,	/* LATIN_CAPITAL_LETTER_K_WITH_CEDILLA */
  GDK_Multi_key,	GDK_L,	GDK_apostrophe,	0,	0,	0x0139,	/* LATIN_CAPITAL_LETTER_L_WITH_ACUTE */
  GDK_Multi_key,	GDK_L,	GDK_comma,	0,	0,	0x013B,	/* LATIN_CAPITAL_LETTER_L_WITH_CEDILLA */
  GDK_Multi_key,	GDK_L,	GDK_minus,	0,	0,	0x00A3,	/* POUND_SIGN */
  GDK_Multi_key,	GDK_L,	GDK_period,	0,	0,	0x013F,	/* LATIN_CAPITAL_LETTER_L_WITH_MIDDLE_DOT */
  GDK_Multi_key,	GDK_L,	GDK_slash,	0,	0,	0x0141,	/* LATIN_CAPITAL_LETTER_L_WITH_STROKE */
  GDK_Multi_key,	GDK_L,	GDK_less,	0,	0,	0x013D,	/* LATIN_CAPITAL_LETTER_L_WITH_CARON */
  GDK_Multi_key,	GDK_L,	GDK_cedilla,	0,	0,	0x013B,	/* LATIN_CAPITAL_LETTER_L_WITH_CEDILLA */
  GDK_Multi_key,	GDK_L,	GDK_equal,	0,	0,	0x00A3,	/* POUND_SIGN */
  GDK_Multi_key,	GDK_L,	GDK_V,	0,	0,	0x007C,	/* VERTICAL_LINE */
  GDK_Multi_key,	GDK_M,	GDK_period,	0,	0,	0x1E40,	/* LATIN_CAPITAL_LETTER_M_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_M,	GDK_slash,	0,	0,	0x20A5,	/* MILL_SIGN */
  GDK_Multi_key,	GDK_N,	GDK_apostrophe,	0,	0,	0x0143,	/* LATIN_CAPITAL_LETTER_N_WITH_ACUTE */
  GDK_Multi_key,	GDK_N,	GDK_comma,	0,	0,	0x0145,	/* LATIN_CAPITAL_LETTER_N_WITH_CEDILLA */
  GDK_Multi_key,	GDK_N,	GDK_minus,	0,	0,	0x00D1,	/* LATIN_CAPITAL_LETTER_N_WITH_TILDE */
  GDK_Multi_key,	GDK_N,	GDK_less,	0,	0,	0x0147,	/* LATIN_CAPITAL_LETTER_N_WITH_CARON */
  GDK_Multi_key,	GDK_N,	GDK_G,	0,	0,	0x014A,	/* LATIN_CAPITAL_LETTER_ENG */
  GDK_Multi_key,	GDK_N,	GDK_N,	0,	0,	0x2229,	/* INTERSECTION */
  GDK_Multi_key,	GDK_N,	GDK_O,	0,	0,	0x2116,	/* NUMERO_SIGN */
  GDK_Multi_key,	GDK_N,	GDK_o,	0,	0,	0x2116,	/* NUMERO_SIGN */
  GDK_Multi_key,	GDK_O,	GDK_plus,	0,	0,	0x2295,	/* CIRCLED_PLUS */
  GDK_Multi_key,	GDK_O,	GDK_comma,	0,	0,	0x01EA,	/* LATIN_CAPITAL_LETTER_O_WITH_OGONEK */
  GDK_Multi_key,	GDK_O,	GDK_minus,	0,	0,	0x014C,	/* LATIN_CAPITAL_LETTER_O_WITH_MACRON */
  GDK_Multi_key,	GDK_O,	GDK_slash,	0,	0,	0x00D8,	/* LATIN_CAPITAL_LETTER_O_WITH_STROKE */
  GDK_Multi_key,	GDK_O,	GDK_colon,	0,	0,	0x0150,	/* LATIN_CAPITAL_LETTER_O_WITH_DOUBLE_ACUTE */
  GDK_Multi_key,	GDK_O,	GDK_greater,	0,	0,	0x00D4,	/* LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_O,	GDK_C,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_O,	GDK_E,	0,	0,	0x0152,	/* LATIN_CAPITAL_LIGATURE_OE */
  GDK_Multi_key,	GDK_O,	GDK_P,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_O,	GDK_R,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_O,	GDK_S,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_O,	GDK_X,	0,	0,	0x2297,	/* CIRCLED_TIMES */
  GDK_Multi_key,	GDK_O,	GDK_asciicircum,	0,	0,	0x00D4,	/* LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_O,	GDK_underscore,	0,	0,	0x00BA,	/* MASCULINE_ORDINAL_INDICATOR */
  GDK_Multi_key,	GDK_O,	GDK_grave,	0,	0,	0x00D2,	/* LATIN_CAPITAL_LETTER_O_WITH_GRAVE */
  GDK_Multi_key,	GDK_O,	GDK_c,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_O,	GDK_p,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_O,	GDK_r,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_O,	GDK_s,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_O,	GDK_x,	0,	0,	0x2297,	/* CIRCLED_TIMES */
  GDK_Multi_key,	GDK_O,	GDK_asciitilde,	0,	0,	0x00D5,	/* LATIN_CAPITAL_LETTER_O_WITH_TILDE */
  GDK_Multi_key,	GDK_O,	GDK_diaeresis,	0,	0,	0x00D6,	/* LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_O,	GDK_acute,	0,	0,	0x00D3,	/* LATIN_CAPITAL_LETTER_O_WITH_ACUTE */
  GDK_Multi_key,	GDK_O,	GDK_breve,	0,	0,	0x014E,	/* LATIN_CAPITAL_LETTER_O_WITH_BREVE */
  GDK_Multi_key,	GDK_P,	GDK_exclam,	0,	0,	0x00B6,	/* PILCROW_SIGN */
  GDK_Multi_key,	GDK_P,	GDK_period,	0,	0,	0x1E56,	/* LATIN_CAPITAL_LETTER_P_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_P,	GDK_0,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_P,	GDK_D,	0,	0,	0x2202,	/* PARTIAL_DIFFERENTIAL */
  GDK_Multi_key,	GDK_P,	GDK_O,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_P,	GDK_T,	0,	0,	0x20A7,	/* PESETA_SIGN */
  GDK_Multi_key,	GDK_P,	GDK_o,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_P,	GDK_t,	0,	0,	0x20A7,	/* PESETA_SIGN */
  GDK_Multi_key,	GDK_P,	GDK_bar,	0,	0,	0x00B6,	/* PILCROW_SIGN */
  GDK_Multi_key,	GDK_R,	GDK_apostrophe,	0,	0,	0x0154,	/* LATIN_CAPITAL_LETTER_R_WITH_ACUTE */
  GDK_Multi_key,	GDK_R,	GDK_comma,	0,	0,	0x0156,	/* LATIN_CAPITAL_LETTER_R_WITH_CEDILLA */
  GDK_Multi_key,	GDK_R,	GDK_0,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_R,	GDK_less,	0,	0,	0x0158,	/* LATIN_CAPITAL_LETTER_R_WITH_CARON */
  GDK_Multi_key,	GDK_R,	GDK_O,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_R,	GDK_S,	0,	0,	0x20A8,	/* RUPEE_SIGN */
  GDK_Multi_key,	GDK_R,	GDK_X,	0,	0,	0x211E,	/* PRESCRIPTION_TAKE */
  GDK_Multi_key,	GDK_R,	GDK_o,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_R,	GDK_s,	0,	0,	0x20A8,	/* RUPEE_SIGN */
  GDK_Multi_key,	GDK_R,	GDK_x,	0,	0,	0x211E,	/* PRESCRIPTION_TAKE */
  GDK_Multi_key,	GDK_R,	GDK_acute,	0,	0,	0x0154,	/* LATIN_CAPITAL_LETTER_R_WITH_ACUTE */
  GDK_Multi_key,	GDK_R,	GDK_cedilla,	0,	0,	0x0156,	/* LATIN_CAPITAL_LETTER_R_WITH_CEDILLA */
  GDK_Multi_key,	GDK_S,	GDK_exclam,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_S,	GDK_apostrophe,	0,	0,	0x015A,	/* LATIN_CAPITAL_LETTER_S_WITH_ACUTE */
  GDK_Multi_key,	GDK_S,	GDK_comma,	0,	0,	0x015E,	/* LATIN_CAPITAL_LETTER_S_WITH_CEDILLA */
  GDK_Multi_key,	GDK_S,	GDK_period,	0,	0,	0x1E60,	/* LATIN_CAPITAL_LETTER_S_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_S,	GDK_0,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_S,	GDK_4,	0,	0,	0x2074,	/* SUPERSCRIPT_FOUR */
  GDK_Multi_key,	GDK_S,	GDK_5,	0,	0,	0x2075,	/* SUPERSCRIPT_FIVE */
  GDK_Multi_key,	GDK_S,	GDK_6,	0,	0,	0x2076,	/* SUPERSCRIPT_SIX */
  GDK_Multi_key,	GDK_S,	GDK_7,	0,	0,	0x2077,	/* SUPERSCRIPT_SEVEN */
  GDK_Multi_key,	GDK_S,	GDK_8,	0,	0,	0x2078,	/* SUPERSCRIPT_EIGHT */
  GDK_Multi_key,	GDK_S,	GDK_9,	0,	0,	0x2079,	/* SUPERSCRIPT_NINE */
  GDK_Multi_key,	GDK_S,	GDK_less,	0,	0,	0x0160,	/* LATIN_CAPITAL_LETTER_S_WITH_CARON */
  GDK_Multi_key,	GDK_S,	GDK_greater,	0,	0,	0x015C,	/* LATIN_CAPITAL_LETTER_S_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_S,	GDK_M,	0,	0,	0x2120,	/* SERVICE MARK */
  GDK_Multi_key,	GDK_S,	GDK_O,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_T,	GDK_H,	0,	0,	0x00DE,	/* LATIN_CAPITAL_LETTER_THORN */
  GDK_Multi_key,	GDK_U,	GDK_slash,	0,	0,	0x00B5,	/* MICRO_SIGN */
  GDK_Multi_key,	GDK_U,	GDK_colon,	0,	0,	0x0170,	/* LATIN_CAPITAL_LETTER_U_WITH_DOUBLE_ACUTE */
  GDK_Multi_key,	GDK_U,	GDK_greater,	0,	0,	0x00DB,	/* LATIN_CAPITAL_LETTER_U_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_U,	GDK_U,	0,	0,	0x222A,	/* UNION */
  GDK_Multi_key,	GDK_U,	GDK_diaeresis,	0,	0,	0x00DC,	/* LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_U,	GDK_acute,	0,	0,	0x00DA,	/* LATIN_CAPITAL_LETTER_U_WITH_ACUTE */
  GDK_Multi_key,	GDK_U,	GDK_breve,	0,	0,	0x016C,	/* LATIN_CAPITAL_LETTER_U_WITH_BREVE */
  GDK_Multi_key,	GDK_V,	GDK_slash,	0,	0,	0x2123,	/* VERSICLE */
  GDK_Multi_key,	GDK_V,	GDK_L,	0,	0,	0x007C,	/* VERTICAL_LINE */
  GDK_Multi_key,	GDK_W,	GDK_quotedbl,	0,	0,	0x1E84,	/* LATIN_CAPITAL_LETTER_W_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_W,	GDK_greater,	0,	0,	0x0174,	/* LATIN_CAPITAL_LETTER_W_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_W,	GDK_asciicircum,	0,	0,	0x0174,	/* LATIN_CAPITAL_LETTER_W_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_W,	GDK_diaeresis,	0,	0,	0x1E84,	/* LATIN_CAPITAL_LETTER_W_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_X,	GDK_0,	0,	0,	0x00A4,	/* CURRENCY_SIGN */
  GDK_Multi_key,	GDK_X,	GDK_O,	0,	0,	0x00A4,	/* CURRENCY_SIGN */
  GDK_Multi_key,	GDK_X,	GDK_o,	0,	0,	0x00A4,	/* CURRENCY_SIGN */
  GDK_Multi_key,	GDK_bracketleft,	GDK_minus,	0,	0,	0x2208,	/* ELEMENT_OF */
  GDK_Multi_key,	GDK_bracketleft,	GDK_bracketleft,	0,	0,	0x2282,	/* SUBSET_OF */
  GDK_Multi_key,	GDK_bracketleft,	GDK_bracketright,	0,	0,	0x25A1,	/* WHITE_SQUARE */
  GDK_Multi_key,	GDK_bracketleft,	GDK_underscore,	0,	0,	0x2286,	/* SUBSET_OF_OR_EQUAL_TO */
  GDK_Multi_key,	GDK_backslash,	GDK_slash,	0,	0,	0x2228,	/* LOGICAL_OR */
  GDK_Multi_key,	GDK_bracketright,	GDK_minus,	0,	0,	0x220B,	/* CONTAINS_AS_MEMBER */
  GDK_Multi_key,	GDK_bracketright,	GDK_bracketright,	0,	0,	0x2283,	/* SUPERSET_OF */
  GDK_Multi_key,	GDK_bracketright,	GDK_underscore,	0,	0,	0x2287,	/* SUPERSET_OF_OR_EQUAL_TO */
  GDK_Multi_key,	GDK_asciicircum,	GDK_space,	0,	0,	0x005E,	/* CIRCUMFLEX_ACCENT */
  GDK_Multi_key,	GDK_asciicircum,	GDK_minus,	0,	0,	0x00AF,	/* MACRON */
  GDK_Multi_key,	GDK_asciicircum,	GDK_period,	0,	0,	0x02D9,	/* DOT_ABOVE */
  GDK_Multi_key,	GDK_asciicircum,	GDK_slash,	0,	0,	0x007C,	/* VERTICAL_LINE */
  GDK_Multi_key,	GDK_asciicircum,	GDK_0,	0,	0,	0x00B0,	/* DEGREE_SIGN */
  GDK_Multi_key,	GDK_asciicircum,	GDK_1,	0,	0,	0x00B9,	/* SUPERSCRIPT_ONE */
  GDK_Multi_key,	GDK_asciicircum,	GDK_2,	0,	0,	0x00B2,	/* SUPERSCRIPT_TWO */
  GDK_Multi_key,	GDK_asciicircum,	GDK_3,	0,	0,	0x00B3,	/* SUPERSCRIPT_THREE */
  GDK_Multi_key,	GDK_asciicircum,	GDK_4,	0,	0,	0x2074,	/* SUPERSCRIPT_FOUR */
  GDK_Multi_key,	GDK_asciicircum,	GDK_5,	0,	0,	0x2075,	/* SUPERSCRIPT_FIVE */
  GDK_Multi_key,	GDK_asciicircum,	GDK_6,	0,	0,	0x2076,	/* SUPERSCRIPT_SIX */
  GDK_Multi_key,	GDK_asciicircum,	GDK_7,	0,	0,	0x2077,	/* SUPERSCRIPT_SEVEN */
  GDK_Multi_key,	GDK_asciicircum,	GDK_8,	0,	0,	0x2078,	/* SUPERSCRIPT_EIGHT */
  GDK_Multi_key,	GDK_asciicircum,	GDK_9,	0,	0,	0x2079,	/* SUPERSCRIPT_NINE */
  GDK_Multi_key,	GDK_asciicircum,	GDK_underscore,	0,	0,	0x00AF,	/* MACRON */
  GDK_Multi_key,	GDK_asciicircum,	GDK_v,	0,	0,	0x2195,	/* UP_DOWN_ARROW */
  GDK_Multi_key,	GDK_asciicircum,	GDK_w,	0,	0,	0x0175,	/* LATIN_SMALL_LETTER_W_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_asciicircum,	GDK_y,	0,	0,	0x0177,	/* LATIN_SMALL_LETTER_Y_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_asciicircum,	GDK_bar,	0,	0,	0x2191,	/* UPWARDS_ARROW */
  GDK_Multi_key,	GDK_asciicircum,	GDK_Left,	0,	0,	0x0302,	/* COMBINING_CIRCUMFLEX_ACCENT */
  GDK_Multi_key,	GDK_asciicircum,	GDK_KP_Left,	0,	0,	0x0302,	/* COMBINING_CIRCUMFLEX_ACCENT */
  GDK_Multi_key,	GDK_underscore,	GDK_slash,	0,	0,	0x2220,	/* ANGLE */
  GDK_Multi_key,	GDK_underscore,	GDK_0,	0,	0,	0x2080,	/* SUBSCRIPT_ZERO */
  GDK_Multi_key,	GDK_underscore,	GDK_1,	0,	0,	0x2081,	/* SUBSCRIPT_ONE */
  GDK_Multi_key,	GDK_underscore,	GDK_2,	0,	0,	0x2082,	/* SUBSCRIPT_TWO */
  GDK_Multi_key,	GDK_underscore,	GDK_3,	0,	0,	0x2083,	/* SUBSCRIPT_THREE */
  GDK_Multi_key,	GDK_underscore,	GDK_4,	0,	0,	0x2084,	/* SUBSCRIPT_FOUR */
  GDK_Multi_key,	GDK_underscore,	GDK_5,	0,	0,	0x2085,	/* SUBSCRIPT_FIVE */
  GDK_Multi_key,	GDK_underscore,	GDK_6,	0,	0,	0x2086,	/* SUBSCRIPT_SIX */
  GDK_Multi_key,	GDK_underscore,	GDK_7,	0,	0,	0x2087,	/* SUBSCRIPT_SEVEN */
  GDK_Multi_key,	GDK_underscore,	GDK_8,	0,	0,	0x2088,	/* SUBSCRIPT_EIGHT */
  GDK_Multi_key,	GDK_underscore,	GDK_9,	0,	0,	0x2089,	/* SUBSCRIPT_NINE */
  GDK_Multi_key,	GDK_underscore,	GDK_A,	0,	0,	0x00AA,	/* FEMININE_ORDINAL_INDICATOR */
  GDK_Multi_key,	GDK_underscore,	GDK_E,	0,	0,	0x0112,	/* LATIN_CAPITAL_LETTER_E_WITH_MACRON */
  GDK_Multi_key,	GDK_underscore,	GDK_I,	0,	0,	0x012A,	/* LATIN_CAPITAL_LETTER_I_WITH_MACRON */
  GDK_Multi_key,	GDK_underscore,	GDK_O,	0,	0,	0x00BA,	/* MASCULINE_ORDINAL_INDICATOR */
  GDK_Multi_key,	GDK_underscore,	GDK_U,	0,	0,	0x016A,	/* LATIN_CAPITAL_LETTER_U_WITH_MACRON */
  GDK_Multi_key,	GDK_underscore,	GDK_bracketleft,	0,	0,	0x2286,	/* SUBSET_OF_OR_EQUAL_TO */
  GDK_Multi_key,	GDK_underscore,	GDK_bracketright,	0,	0,	0x2287,	/* SUPERSET_OF_OR_EQUAL_TO */
  GDK_Multi_key,	GDK_underscore,	GDK_asciicircum,	0,	0,	0x00AF,	/* MACRON */
  GDK_Multi_key,	GDK_underscore,	GDK_underscore,	0,	0,	0x00AF,	/* MACRON */
  GDK_Multi_key,	GDK_underscore,	GDK_a,	0,	0,	0x00AA,	/* FEMININE_ORDINAL_INDICATOR */
  GDK_Multi_key,	GDK_underscore,	GDK_e,	0,	0,	0x0113,	/* LATIN_SMALL_LETTER_E_WITH_MACRON */
  GDK_Multi_key,	GDK_underscore,	GDK_i,	0,	0,	0x012B,	/* LATIN_SMALL_LETTER_I_WITH_MACRON */
  GDK_Multi_key,	GDK_underscore,	GDK_o,	0,	0,	0x00BA,	/* MASCULINE_ORDINAL_INDICATOR */
  GDK_Multi_key,	GDK_underscore,	GDK_u,	0,	0,	0x016B,	/* LATIN_SMALL_LETTER_U_WITH_MACRON */
  GDK_Multi_key,	GDK_underscore,	GDK_bar,	0,	0,	0x22A5,	/* UP_TACK */
  GDK_Multi_key,	GDK_underscore,	GDK_Left,	0,	0,	0x0332,	/* COMBINING_LOW_LINE */
  GDK_Multi_key,	GDK_underscore,	GDK_KP_Left,	0,	0,	0x0332,	/* COMBINING_LOW_LINE */
  GDK_Multi_key,	GDK_grave,	GDK_space,	0,	0,	0x0060,	/* GRAVE_ACCENT */
  GDK_Multi_key,	GDK_grave,	GDK_A,	0,	0,	0x00C0,	/* LATIN_CAPITAL_LETTER_A_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_E,	0,	0,	0x00C8,	/* LATIN_CAPITAL_LETTER_E_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_I,	0,	0,	0x00CC,	/* LATIN_CAPITAL_LETTER_I_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_O,	0,	0,	0x00D2,	/* LATIN_CAPITAL_LETTER_O_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_U,	0,	0,	0x00D9,	/* LATIN_CAPITAL_LETTER_U_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_Y,	0,	0,	0x1EF2,	/* LATIN_CAPITAL_LETTER_Y_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_a,	0,	0,	0x00E0,	/* LATIN_SMALL_LETTER_A_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_e,	0,	0,	0x00E8,	/* LATIN_SMALL_LETTER_E_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_i,	0,	0,	0x00EC,	/* LATIN_SMALL_LETTER_I_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_o,	0,	0,	0x00F2,	/* LATIN_SMALL_LETTER_O_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_u,	0,	0,	0x00F9,	/* LATIN_SMALL_LETTER_U_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_y,	0,	0,	0x1EF3,	/* LATIN_SMALL_LETTER_Y_WITH_GRAVE */
  GDK_Multi_key,	GDK_grave,	GDK_Left,	0,	0,	0x0300,	/* COMBINING_GRAVE_ACCENT */
  GDK_Multi_key,	GDK_grave,	GDK_KP_Left,	0,	0,	0x0300,	/* COMBINING_GRAVE_ACCENT */
  GDK_Multi_key,	GDK_a,	GDK_quotedbl,	0,	0,	0x00E4,	/* LATIN_SMALL_LETTER_A_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_a,	GDK_apostrophe,	0,	0,	0x00E1,	/* LATIN_SMALL_LETTER_A_WITH_ACUTE */
  GDK_Multi_key,	GDK_a,	GDK_parenleft,	0,	0,	0x0103,	/* LATIN_SMALL_LETTER_A_WITH_BREVE */
  GDK_Multi_key,	GDK_a,	GDK_comma,	0,	0,	0x0105,	/* LATIN_SMALL_LETTER_A_WITH_OGONEK */
  GDK_Multi_key,	GDK_a,	GDK_asterisk,	0,	0,	0x00E5,	/* LATIN_SMALL_LETTER_A_WITH_RING_ABOVE */
  GDK_Multi_key,	GDK_a,	GDK_minus,	0,	0,	0x0101,	/* LATIN_SMALL_LETTER_A_WITH_MACRON */
  GDK_Multi_key,	GDK_a,	GDK_greater,	0,	0,	0x00E2,	/* LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_a,	GDK_asciicircum,	0,	0,	0x00E2,	/* LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_a,	GDK_underscore,	0,	0,	0x00AA,	/* FEMININE_ORDINAL_INDICATOR */
  GDK_Multi_key,	GDK_a,	GDK_grave,	0,	0,	0x00E0,	/* LATIN_SMALL_LETTER_A_WITH_GRAVE */
  GDK_Multi_key,	GDK_a,	GDK_a,	0,	0,	0x00E5,	/* LATIN_SMALL_LETTER_A_WITH_RING_ABOVE */
  GDK_Multi_key,	GDK_a,	GDK_e,	0,	0,	0x00E6,	/* LATIN_SMALL_LETTER_AE */
  GDK_Multi_key,	GDK_a,	GDK_asciitilde,	0,	0,	0x00E3,	/* LATIN_SMALL_LETTER_A_WITH_TILDE */
  GDK_Multi_key,	GDK_a,	GDK_diaeresis,	0,	0,	0x00E4,	/* LATIN_SMALL_LETTER_A_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_a,	GDK_acute,	0,	0,	0x00E1,	/* LATIN_SMALL_LETTER_A_WITH_ACUTE */
  GDK_Multi_key,	GDK_b,	GDK_period,	0,	0,	0x1E03,	/* LATIN_SMALL_LETTER_B_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_b,	GDK_b,	0,	0,	0x266D,	/* MUSIC_FLAT_SIGN */
  GDK_Multi_key,	GDK_c,	GDK_apostrophe,	0,	0,	0x0107,	/* LATIN_SMALL_LETTER_C_WITH_ACUTE */
  GDK_Multi_key,	GDK_c,	GDK_comma,	0,	0,	0x00E7,	/* LATIN_SMALL_LETTER_C_WITH_CEDILLA */
  GDK_Multi_key,	GDK_c,	GDK_period,	0,	0,	0x010B,	/* LATIN_SMALL_LETTER_C_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_c,	GDK_slash,	0,	0,	0x00A2,	/* CENT_SIGN */
  GDK_Multi_key,	GDK_c,	GDK_0,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_c,	GDK_less,	0,	0,	0x010D,	/* LATIN_SMALL_LETTER_C_WITH_CARON */
  GDK_Multi_key,	GDK_c,	GDK_greater,	0,	0,	0x0109,	/* LATIN_SMALL_LETTER_C_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_c,	GDK_O,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_c,	GDK_asciicircum,	0,	0,	0x0109,	/* LATIN_SMALL_LETTER_C_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_c,	GDK_l,	0,	0,	0x2104,	/* CENTRE_LINE_SYMBOL */
  GDK_Multi_key,	GDK_c,	GDK_o,	0,	0,	0x2105,	/* CARE_OF */
  GDK_Multi_key,	GDK_c,	GDK_r,	0,	0,	0x20A2,	/* CRUZEIRO_SIGN */
  GDK_Multi_key,	GDK_c,	GDK_u,	0,	0,	0x2106,	/* CADA_UNA */
  GDK_Multi_key,	GDK_c,	GDK_bar,	0,	0,	0x00A2,	/* CENT_SIGN */
  GDK_Multi_key,	GDK_d,	GDK_minus,	0,	0,	0x0111,	/* LATIN_SMALL_LETTER_D_WITH_STROKE */
  GDK_Multi_key,	GDK_d,	GDK_period,	0,	0,	0x1E0B,	/* LATIN_SMALL_LETTER_D_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_d,	GDK_slash,	0,	0,	0x00F0,	/* LATIN_SMALL_LETTER_ETH */
  GDK_Multi_key,	GDK_d,	GDK_less,	0,	0,	0x010F,	/* LATIN_SMALL_LETTER_D_WITH_CARON */
  GDK_Multi_key,	GDK_e,	GDK_parenleft,	0,	0,	0x0115,	/* LATIN_SMALL_LETTER_E_WITH_BREVE */
  GDK_Multi_key,	GDK_e,	GDK_equal,	0,	0,	0x20AC,	/* EURO_SIGN */
  GDK_Multi_key,	GDK_e,	GDK_equal,	0,	0,	0x20AC,	/* EURO_SIGN */
  GDK_Multi_key,	GDK_e,	GDK_greater,	0,	0,	0x00EA,	/* LATIN_SMALL_LETTER_E_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_e,	GDK_asciicircum,	0,	0,	0x00EA,	/* LATIN_SMALL_LETTER_E_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_e,	GDK_underscore,	0,	0,	0x0113,	/* LATIN_SMALL_LETTER_E_WITH_MACRON */
  GDK_Multi_key,	GDK_e,	GDK_grave,	0,	0,	0x00E8,	/* LATIN_SMALL_LETTER_E_WITH_GRAVE */
  GDK_Multi_key,	GDK_e,	GDK_e,	0,	0,	0x0259,	/* LATIN_SMALL_LETTER_SCHWA */
  GDK_Multi_key,	GDK_g,	GDK_U,	0,	0,	0x011F,	/* LATIN_SMALL_LETTER_G_WITH_BREVE */
  GDK_Multi_key,	GDK_g,	GDK_asciicircum,	0,	0,	0x011D,	/* LATIN_SMALL_LETTER_G_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_g,	GDK_breve,	0,	0,	0x011F,	/* LATIN_SMALL_LETTER_G_WITH_BREVE */
  GDK_Multi_key,	GDK_h,	GDK_minus,	0,	0,	0x0127,	/* LATIN_SMALL_LETTER_H_WITH_STROKE */
  GDK_Multi_key,	GDK_h,	GDK_slash,	0,	0,	0x210F,	/* PLANCK_CONSTANT_OVER_TWO_PI */
  GDK_Multi_key,	GDK_i,	GDK_j,	0,	0,	0x0133,	/* LATIN_SMALL_LIGATURE_IJ */
  GDK_Multi_key,	GDK_k,	GDK_k,	0,	0,	0x0138,	/* LATIN_SMALL_LETTER_KRA */
  GDK_Multi_key,	GDK_l,	GDK_apostrophe,	0,	0,	0x013A,	/* LATIN_SMALL_LETTER_L_WITH_ACUTE */
  GDK_Multi_key,	GDK_l,	GDK_comma,	0,	0,	0x013C,	/* LATIN_SMALL_LETTER_L_WITH_CEDILLA */
  GDK_Multi_key,	GDK_l,	GDK_minus,	0,	0,	0x00A3,	/* POUND_SIGN */
  GDK_Multi_key,	GDK_l,	GDK_period,	0,	0,	0x0140,	/* LATIN_SMALL_LETTER_L_WITH_MIDDLE_DOT */
  GDK_Multi_key,	GDK_l,	GDK_slash,	0,	0,	0x0142,	/* LATIN_SMALL_LETTER_L_WITH_STROKE */
  GDK_Multi_key,	GDK_l,	GDK_less,	0,	0,	0x013E,	/* LATIN_SMALL_LETTER_L_WITH_CARON */
  GDK_Multi_key,	GDK_l,	GDK_equal,	0,	0,	0x00A3,	/* POUND_SIGN */
  GDK_Multi_key,	GDK_l,	GDK_v,	0,	0,	0x007C,	/* VERTICAL_LINE */
  GDK_Multi_key,	GDK_m,	GDK_minus,	0,	0,	0x2014,	/* EM_DASH */
  GDK_Multi_key,	GDK_m,	GDK_period,	0,	0,	0x1E41,	/* LATIN_SMALL_LETTER_M_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_m,	GDK_slash,	0,	0,	0x20A5,	/* MILL_SIGN */
  GDK_Multi_key,	GDK_n,	GDK_apostrophe,	0,	0,	0x0144,	/* LATIN_SMALL_LETTER_N_WITH_ACUTE */
  GDK_Multi_key,	GDK_n,	GDK_comma,	0,	0,	0x0146,	/* LATIN_SMALL_LETTER_N_WITH_CEDILLA */
  GDK_Multi_key,	GDK_n,	GDK_minus,	0,	0,	0x00F1,	/* LATIN_SMALL_LETTER_N_WITH_TILDE */
  GDK_Multi_key,	GDK_n,	GDK_less,	0,	0,	0x0148,	/* LATIN_SMALL_LETTER_N_WITH_CARON */
  GDK_Multi_key,	GDK_n,	GDK_g,	0,	0,	0x014B,	/* LATIN_SMALL_LETTER_ENG */
  GDK_Multi_key,	GDK_n,	GDK_n,	0,	0,	0x266E,	/* MUSIC_NATURAL_SIGN */
  GDK_Multi_key,	GDK_n,	GDK_o,	0,	0,	0x2116,	/* NUMERO_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_parenleft,	0,	0,	0x014F,	/* LATIN_SMALL_LETTER_O_WITH_BREVE */
  GDK_Multi_key,	GDK_o,	GDK_plus,	0,	0,	0x2295,	/* CIRCLED_PLUS */
  GDK_Multi_key,	GDK_o,	GDK_comma,	0,	0,	0x01EB,	/* LATIN_SMALL_LETTER_O_WITH_OGONEK */
  GDK_Multi_key,	GDK_o,	GDK_colon,	0,	0,	0x0151,	/* LATIN_SMALL_LETTER_O_WITH_DOUBLE_ACUTE */
  GDK_Multi_key,	GDK_o,	GDK_greater,	0,	0,	0x00F4,	/* LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_o,	GDK_C,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_P,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_o,	GDK_R,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_S,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_X,	0,	0,	0x00A4,	/* CURRENCY_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_asciicircum,	0,	0,	0x00F4,	/* LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_o,	GDK_underscore,	0,	0,	0x00BA,	/* MASCULINE_ORDINAL_INDICATOR */
  GDK_Multi_key,	GDK_o,	GDK_grave,	0,	0,	0x00F2,	/* LATIN_SMALL_LETTER_O_WITH_GRAVE */
  GDK_Multi_key,	GDK_o,	GDK_c,	0,	0,	0x00A9,	/* COPYRIGHT_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_e,	0,	0,	0x0153,	/* LATIN_SMALL_LIGATURE_OE */
  GDK_Multi_key,	GDK_o,	GDK_p,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_o,	GDK_r,	0,	0,	0x00AE,	/* REGISTERED_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_s,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_x,	0,	0,	0x00A4,	/* CURRENCY_SIGN */
  GDK_Multi_key,	GDK_o,	GDK_breve,	0,	0,	0x014F,	/* LATIN_SMALL_LETTER_O_WITH_BREVE */
  GDK_Multi_key,	GDK_p,	GDK_exclam,	0,	0,	0x00B6,	/* PILCROW_SIGN */
  GDK_Multi_key,	GDK_p,	GDK_0,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_p,	GDK_O,	0,	0,	0x2117,	/* SOUND_RECORDING_COPYRIGHT */
  GDK_Multi_key,	GDK_p,	GDK_d,	0,	0,	0x2202,	/* PARTIAL_DIFFERENTIAL */
  GDK_Multi_key,	GDK_p,	GDK_t,	0,	0,	0x20A7,	/* PESETA_SIGN */
  GDK_Multi_key,	GDK_p,	GDK_bar,	0,	0,	0x00B6,	/* PILCROW_SIGN */
  GDK_Multi_key,	GDK_r,	GDK_s,	0,	0,	0x20A8,	/* RUPEE_SIGN */
  GDK_Multi_key,	GDK_s,	GDK_minus,	0,	0,	0x017F,	/* LATIN_SMALL_LETTER_LONG_S */
  GDK_Multi_key,	GDK_s,	GDK_period,	0,	0,	0x1E61,	/* LATIN_SMALL_LETTER_S_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_s,	GDK_0,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_s,	GDK_4,	0,	0,	0x2074,	/* SUPERSCRIPT_FOUR */ /* these are still missing from simple im */
  GDK_Multi_key,	GDK_s,	GDK_5,	0,	0,	0x2075,	/* SUPERSCRIPT_FIVE */
  GDK_Multi_key,	GDK_s,	GDK_6,	0,	0,	0x2076,	/* SUPERSCRIPT_SIX */
  GDK_Multi_key,	GDK_s,	GDK_7,	0,	0,	0x2077,	/* SUPERSCRIPT_SEVEN */
  GDK_Multi_key,	GDK_s,	GDK_8,	0,	0,	0x2078,	/* SUPERSCRIPT_EIGHT */
  GDK_Multi_key,	GDK_s,	GDK_9,	0,	0,	0x2079,	/* SUPERSCRIPT_NINE */
  GDK_Multi_key,	GDK_s,	GDK_less,	0,	0,	0x0161,	/* LATIN_SMALL_LETTER_S_WITH_CARON */
  GDK_Multi_key,	GDK_s,	GDK_greater,	0,	0,	0x015D,	/* LATIN_SMALL_LETTER_S_WITH_CIRCUMFLEX */
  GDK_Multi_key,	GDK_s,	GDK_m,	0,	0,	0x2120,	/* SERVICE MARK */
  GDK_Multi_key,	GDK_s,	GDK_o,	0,	0,	0x00A7,	/* SECTION_SIGN */
  GDK_Multi_key,	GDK_s,	GDK_s,	0,	0,	0x00DF,	/* LATIN_SMALL_LETTER_SHARP_S */
  GDK_Multi_key,	GDK_s,	GDK_z,	0,	0,	0x00DF,	/* LATIN_SMALL_LETTER_SHARP_S */ /* Not in simple (I think it was?) */
  GDK_Multi_key,	GDK_u,	GDK_slash,	0,	0,	0x00B5,	/* MICRO_SIGN */
  GDK_Multi_key,	GDK_u,	GDK_colon,	0,	0,	0x0171,	/* LATIN_SMALL_LETTER_U_WITH_DOUBLE_ACUTE */
  GDK_Multi_key,	GDK_v,	GDK_slash,	0,	0,	0x2123,	/* VERSICLE */
  GDK_Multi_key,	GDK_v,	GDK_2,	0,	0,	0x221A,	/* SQUARE ROOT */
  GDK_Multi_key,	GDK_v,	GDK_3,	0,	0,	0x221B,	/* CUBE ROOT */
  GDK_Multi_key,	GDK_v,	GDK_4,	0,	0,	0x221C,	/* FOURTH ROOT */
  GDK_Multi_key,	GDK_v,	GDK_Z,	0,	0,	0x017D,	/* LATIN_CAPITAL_LETTER_Z_WITH_CARON */
  GDK_Multi_key,	GDK_v,	GDK_asciicircum,	0,	0,	0x2195,	/* UP_DOWN_ARROW */
  GDK_Multi_key,	GDK_v,	GDK_l,	0,	0,	0x007C,	/* VERTICAL_LINE */
  GDK_Multi_key,	GDK_v,	GDK_z,	0,	0,	0x017E,	/* LATIN_SMALL_LETTER_Z_WITH_CARON */
  GDK_Multi_key,	GDK_v,	GDK_bar,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
  GDK_Multi_key,	GDK_x,	GDK_0,	0,	0,	0x2297,	/* CIRCLED_TIMES */
  GDK_Multi_key,	GDK_x,	GDK_colon,	0,	0,	0x203B,	/* REFERENCE_MARK */
  GDK_Multi_key,	GDK_x,	GDK_O,	0,	0,	0x2297,	/* CIRCLED_TIMES */
  GDK_Multi_key,	GDK_x,	GDK_o,	0,	0,	0x00A4,	/* CURRENCY_SIGN */
  GDK_Multi_key,	GDK_x,	GDK_x,	0,	0,	0x00D7,	/* MULTIPLICATION_SIGN */
  GDK_Multi_key,	GDK_y,	GDK_grave,	0,	0,	0x1EF3,	/* LATIN_SMALL_LETTER_Y_WITH_GRAVE */
  GDK_Multi_key,	GDK_y,	GDK_diaeresis,	0,	0,	0x00FF,	/* LATIN_SMALL_LETTER_Y_WITH_DIAERESIS */
  GDK_Multi_key,	GDK_y,	GDK_acute,	0,	0,	0x00FD,	/* LATIN_SMALL_LETTER_Y_WITH_ACUTE */
  GDK_Multi_key,	GDK_z,	GDK_apostrophe,	0,	0,	0x017A,	/* LATIN_SMALL_LETTER_Z_WITH_ACUTE */
  GDK_Multi_key,	GDK_z,	GDK_period,	0,	0,	0x017C,	/* LATIN_SMALL_LETTER_Z_WITH_DOT_ABOVE */
  GDK_Multi_key,	GDK_z,	GDK_less,	0,	0,	0x017E,	/* LATIN_SMALL_LETTER_Z_WITH_CARON */
  GDK_Multi_key,	GDK_z,	GDK_acute,	0,	0,	0x017A,	/* LATIN_SMALL_LETTER_Z_WITH_ACUTE */
  GDK_Multi_key,	GDK_bar,	GDK_minus,	0,	0,	0x2020,	/* DAGGER */
  GDK_Multi_key,	GDK_bar,	GDK_slash,	0,	0,	0x2224,	/* DOES_NOT_DIVIDE */
  GDK_Multi_key,	GDK_bar,	GDK_equal,	0,	0,	0x2021,	/* DOUBLE_DAGGER */
  GDK_Multi_key,	GDK_bar,	GDK_greater,	0,	0,	0x2207,	/* NABLA */
  GDK_Multi_key,	GDK_bar,	GDK_C,	0,	0,	0x00A2,	/* CENT_SIGN */
  GDK_Multi_key,	GDK_bar,	GDK_V,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
  GDK_Multi_key,	GDK_bar,	GDK_asciicircum,	0,	0,	0x2191,	/* UPWARDS_ARROW */
  GDK_Multi_key,	GDK_bar,	GDK_underscore,	0,	0,	0x22A5,	/* UP_TACK */
  GDK_Multi_key,	GDK_bar,	GDK_c,	0,	0,	0x00A2,	/* CENT_SIGN */
  GDK_Multi_key,	GDK_bar,	GDK_v,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
  GDK_Multi_key,	GDK_bar,	GDK_bar,	0,	0,	0x2225,	/* PARALLEL_TO */
  GDK_Multi_key,	GDK_bar,	GDK_Left,	0,	0,	0x21F7,	/* LEFTWARDS_ARROW_WITH_VERTICAL_STROKE */
  GDK_Multi_key,	GDK_bar,	GDK_Right,	0,	0,	0x21F8,	/* RIGHTWARDS_ARROW_WITH_VERTICAL_STROKE */
  GDK_Multi_key,	GDK_bar,	GDK_KP_Left,	0,	0,	0x21F7,	/* LEFTWARDS_ARROW_WITH_VERTICAL_STROKE */
  GDK_Multi_key,	GDK_bar,	GDK_KP_Right,	0,	0,	0x21F8,	/* RIGHTWARDS_ARROW_WITH_VERTICAL_STROKE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_space,	0,	0,	0x007E,	/* TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_A,	0,	0,	0x00C3,	/* LATIN_CAPITAL_LETTER_A_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_I,	0,	0,	0x0128,	/* LATIN_CAPITAL_LETTER_I_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_N,	0,	0,	0x00D1,	/* LATIN_CAPITAL_LETTER_N_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_O,	0,	0,	0x00D5,	/* LATIN_CAPITAL_LETTER_O_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_U,	0,	0,	0x0168,	/* LATIN_CAPITAL_LETTER_U_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_a,	0,	0,	0x00E3,	/* LATIN_SMALL_LETTER_A_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_i,	0,	0,	0x0129,	/* LATIN_SMALL_LETTER_I_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_n,	0,	0,	0x00F1,	/* LATIN_SMALL_LETTER_N_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_o,	0,	0,	0x00F5,	/* LATIN_SMALL_LETTER_O_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_u,	0,	0,	0x0169,	/* LATIN_SMALL_LETTER_U_WITH_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_Left,	0,	0,	0x0303,	/* COMBINING_TILDE */
  GDK_Multi_key,	GDK_asciitilde,	GDK_KP_Left,	0,	0,	0x0303,	/* COMBINING_TILDE */
  GDK_Multi_key,	GDK_diaeresis,	GDK_Left,	0,	0,	0x0308,	/* COMBINING_DIAERESIS */
  GDK_Multi_key,	GDK_Left,	GDK_space,	0,	0,	0x2190,	/* LEFTWARDS_ARROW */
  GDK_Multi_key,	GDK_Left,	GDK_quotedbl,	0,	0,	0x0308,	/* COMBINING_DIAERESIS */
  GDK_Multi_key,	GDK_Left,	GDK_apostrophe,	0,	0,	0x0301,	/* COMBINING_ACUTE_ACCENT */
  GDK_Multi_key,	GDK_Left,	GDK_parenleft,	0,	0,	0x0306,	/* COMBINING_BREVE */
  GDK_Multi_key,	GDK_Left,	GDK_asterisk,	0,	0,	0x030A,	/* COMBINING_RING_ABOVE */
  GDK_Multi_key,	GDK_Left,	GDK_comma,	0,	0,	0x0328,	/* COMBINING_OGONEK */
  GDK_Multi_key,	GDK_Left,	GDK_minus,	0,	0,	0x0304,	/* COMBINING_MACRON */
  GDK_Multi_key,	GDK_Left,	GDK_period,	0,	0,	0x0307,	/* COMBINING_DOT_ABOVE */
  GDK_Multi_key,	GDK_Left,	GDK_slash,	0,	0,	0x0337,	/* COMBINING_LONG_SOLIDUS_OVERLAY */
/*  GDK_Multi_key,	GDK_Left,	GDK_slash,	0,	0,	0x219A,	*//* LEFTWARDS_ARROW_WITH_STROKE */
  GDK_Multi_key,	GDK_Left,	GDK_2,	0,	0,	0x21C7,	/* LEFTWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_Left,	GDK_colon,	0,	0,	0x030B,	/* COMBINING_DOUBLE_ACUTE_ACCENT */
  GDK_Multi_key,	GDK_Left,	GDK_less,	0,	0,	0x030C,	/* COMBINING_CARON */
  GDK_Multi_key,	GDK_Left,	GDK_equal,	0,	0,	0x21D0,	/* LEFTWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_Left,	GDK_greater,	0,	0,	0x0302,	/* COMBINING_CIRCUMFLEX_ACCENT */
  GDK_Multi_key,	GDK_Left,	GDK_asciicircum,	0,	0,	0x0302,	/* COMBINING_CIRCUMFLEX_ACCENT */
  GDK_Multi_key,	GDK_Left,	GDK_underscore,	0,	0,	0x0332,	/* COMBINING_LOW_LINE */
  GDK_Multi_key,	GDK_Left,	GDK_grave,	0,	0,	0x0300,	/* COMBINING_GRAVE_ACCENT */
  GDK_Multi_key,	GDK_Left,	GDK_bar,	0,	0,	0x21F7,	/* LEFTWARDS_ARROW_WITH_VERTICAL_STROKE */
  GDK_Multi_key,	GDK_Left,	GDK_asciitilde,	0,	0,	0x0303,	/* COMBINING_TILDE */
  GDK_Multi_key,	GDK_Left,	GDK_diaeresis,	0,	0,	0x0308,	/* COMBINING_DIAERESIS */
  GDK_Multi_key,	GDK_Left,	GDK_Left,	0,	0,	0x2190,	/* LEFTWARDS_ARROW */
  GDK_Multi_key,	GDK_Left,	GDK_Up,	0,	0,	0x2196,	/* NORTH_WEST_ARROW */
  GDK_Multi_key,	GDK_Left,	GDK_Right,	0,	0,	0x2194,	/* LEFT_RIGHT_ARROW */
  GDK_Multi_key,	GDK_Left,	GDK_Down,	0,	0,	0x2199,	/* SOUTH_WEST_ARROW */
  GDK_Multi_key,	GDK_Up,	GDK_space,	0,	0,	0x2191,	/* UPWARDS_ARROW */
  GDK_Multi_key,	GDK_Up,	GDK_2,	0,	0,	0x21C8,	/* UPWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_Up,	GDK_equal,	0,	0,	0x21D1,	/* UPWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_Up,	GDK_Left,	0,	0,	0x2196,	/* NORTH_WEST_ARROW */
  GDK_Multi_key,	GDK_Up,	GDK_Up,	0,	0,	0x2191,	/* UPWARDS_ARROW */
  GDK_Multi_key,	GDK_Up,	GDK_Right,	0,	0,	0x2197,	/* NORTH_EAST_ARROW */
  GDK_Multi_key,	GDK_Up,	GDK_Down,	0,	0,	0x2195,	/* UP_DOWN_ARROW */
  GDK_Multi_key,	GDK_Right,	GDK_space,	0,	0,	0x2192,	/* RIGHTWARDS_ARROW */
  GDK_Multi_key,	GDK_Right,	GDK_slash,	0,	0,	0x219B,	/* RIGHTWARDS_ARROW_WITH_STROKE */
  GDK_Multi_key,	GDK_Right,	GDK_0,	0,	0,	0x21F4,	/* RIGHT_ARROW_WITH_SMALL_CIRCLE */
  GDK_Multi_key,	GDK_Right,	GDK_2,	0,	0,	0x21C9,	/* RIGHTWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_Right,	GDK_3,	0,	0,	0x21F6,	/* THREE_RIGHTWARDS_ARROWS */
  GDK_Multi_key,	GDK_Right,	GDK_equal,	0,	0,	0x21D2,	/* RIGHTWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_Right,	GDK_bar,	0,	0,	0x21F8,	/* RIGHTWARDS_ARROW_WITH_VERTICAL_STROKE */
  GDK_Multi_key,	GDK_Right,	GDK_Left,	0,	0,	0x2194,	/* LEFT_RIGHT_ARROW */
  GDK_Multi_key,	GDK_Right,	GDK_Up,	0,	0,	0x2197,	/* NORTH_EAST_ARROW */
  GDK_Multi_key,	GDK_Right,	GDK_Right,	0,	0,	0x2192,	/* RIGHTWARDS_ARROW */
  GDK_Multi_key,	GDK_Right,	GDK_Down,	0,	0,	0x2198,	/* SOUTH_EAST_ARROW */
  GDK_Multi_key,	GDK_Down,	GDK_space,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
  GDK_Multi_key,	GDK_Down,	GDK_2,	0,	0,	0x21CA,	/* DOWNWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_Down,	GDK_equal,	0,	0,	0x21D3,	/* DOWNWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_Down,	GDK_Left,	0,	0,	0x2199,	/* SOUTH_WEST_ARROW */
  GDK_Multi_key,	GDK_Down,	GDK_Up,	0,	0,	0x2195,	/* UP_DOWN_ARROW */
  GDK_Multi_key,	GDK_Down,	GDK_Right,	0,	0,	0x2198,	/* SOUTH_EAST_ARROW */
  GDK_Multi_key,	GDK_Down,	GDK_Down,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
  GDK_Multi_key,	GDK_KP_Left,	GDK_space,	0,	0,	0x2190,	/* LEFTWARDS_ARROW */
  GDK_Multi_key,	GDK_KP_Left,	GDK_quotedbl,	0,	0,	0x0308,	/* COMBINING_DIAERESIS */
  GDK_Multi_key,	GDK_KP_Left,	GDK_apostrophe,	0,	0,	0x0301,	/* COMBINING_ACUTE_ACCENT */
  GDK_Multi_key,	GDK_KP_Left,	GDK_parenleft,	0,	0,	0x0306,	/* COMBINING_BREVE */
  GDK_Multi_key,	GDK_KP_Left,	GDK_asterisk,	0,	0,	0x030A,	/* COMBINING_RING_ABOVE */
  GDK_Multi_key,	GDK_KP_Left,	GDK_minus,	0,	0,	0x0304,	/* COMBINING_MACRON */
  GDK_Multi_key,	GDK_KP_Left,	GDK_period,	0,	0,	0x0307,	/* COMBINING_DOT_ABOVE */
  GDK_Multi_key,	GDK_KP_Left,	GDK_slash,	0,	0,	0x0337, /* COMBINING_LONG_SOLIDUS_OVERLAY */
/* This conflicts */
/*  GDK_Multi_key,	GDK_KP_Left,	GDK_slash,	0,	0,	0x219A,	*//* LEFTWARDS_ARROW_WITH_STROKE */
  GDK_Multi_key,	GDK_KP_Left,	GDK_2,	0,	0,	0x21C7,	/* LEFTWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_KP_Left,	GDK_colon,	0,	0,	0x030B,	/* COMBINING_DOUBLE_ACUTE_ACCENT */
  GDK_Multi_key,	GDK_KP_Left,	GDK_equal,	0,	0,	0x21D0,	/* LEFTWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_KP_Left,	GDK_asciicircum,	0,	0,	0x0302,	/* COMBINING_CIRCUMFLEX_ACCENT */
  GDK_Multi_key,	GDK_KP_Left,	GDK_underscore,	0,	0,	0x0332,	/* COMBINING_LOW_LINE */
  GDK_Multi_key,	GDK_KP_Left,	GDK_grave,	0,	0,	0x0300,	/* COMBINING_GRAVE_ACCENT */
  GDK_Multi_key,	GDK_KP_Left,	GDK_bar,	0,	0,	0x21F7,	/* LEFTWARDS_ARROW_WITH_VERTICAL_STROKE */
  GDK_Multi_key,	GDK_KP_Left,	GDK_asciitilde,	0,	0,	0x0303,	/* COMBINING_TILDE */
  GDK_Multi_key,	GDK_KP_Left,	GDK_diaeresis,	0,	0,	0x0308,	/* COMBINING_DIAERESIS */
  GDK_Multi_key,	GDK_KP_Left,	GDK_KP_Left,	0,	0,	0x2190,	/* LEFTWARDS_ARROW */
  GDK_Multi_key,	GDK_KP_Left,	GDK_KP_Up,	0,	0,	0x2196,	/* NORTH_WEST_ARROW */
  GDK_Multi_key,	GDK_KP_Left,	GDK_KP_Right,	0,	0,	0x2194,	/* LEFT_RIGHT_ARROW */
  GDK_Multi_key,	GDK_KP_Left,	GDK_KP_Down,	0,	0,	0x2199,	/* SOUTH_WEST_ARROW */
  GDK_Multi_key,	GDK_KP_Up,	GDK_space,	0,	0,	0x2191,	/* UPWARDS_ARROW */
  GDK_Multi_key,	GDK_KP_Up,	GDK_2,	0,	0,	0x21C8,	/* UPWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_KP_Up,	GDK_equal,	0,	0,	0x21D1,	/* UPWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_KP_Up,	GDK_KP_Left,	0,	0,	0x2196,	/* NORTH_WEST_ARROW */
  GDK_Multi_key,	GDK_KP_Up,	GDK_KP_Up,	0,	0,	0x2191,	/* UPWARDS_ARROW */
  GDK_Multi_key,	GDK_KP_Up,	GDK_KP_Right,	0,	0,	0x2197,	/* NORTH_EAST_ARROW */
  GDK_Multi_key,	GDK_KP_Up,	GDK_KP_Down,	0,	0,	0x2195,	/* UP_DOWN_ARROW */
  GDK_Multi_key,	GDK_KP_Right,	GDK_space,	0,	0,	0x2192,	/* RIGHTWARDS_ARROW */
  GDK_Multi_key,	GDK_KP_Right,	GDK_slash,	0,	0,	0x219B,	/* RIGHTWARDS_ARROW_WITH_STROKE */
  GDK_Multi_key,	GDK_KP_Right,	GDK_0,	0,	0,	0x21F4,	/* RIGHT_ARROW_WITH_SMALL_CIRCLE */
  GDK_Multi_key,	GDK_KP_Right,	GDK_2,	0,	0,	0x21C9,	/* RIGHTWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_KP_Right,	GDK_3,	0,	0,	0x21F6,	/* THREE_RIGHTWARDS_ARROWS */
  GDK_Multi_key,	GDK_KP_Right,	GDK_equal,	0,	0,	0x21D2,	/* RIGHTWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_KP_Right,	GDK_bar,	0,	0,	0x21F8,	/* RIGHTWARDS_ARROW_WITH_VERTICAL_STROKE */
  GDK_Multi_key,	GDK_KP_Right,	GDK_KP_Left,	0,	0,	0x2194,	/* LEFT_RIGHT_ARROW */
  GDK_Multi_key,	GDK_KP_Right,	GDK_KP_Up,	0,	0,	0x2197,	/* NORTH_EAST_ARROW */
  GDK_Multi_key,	GDK_KP_Right,	GDK_KP_Right,	0,	0,	0x2192,	/* RIGHTWARDS_ARROW */
  GDK_Multi_key,	GDK_KP_Right,	GDK_KP_Down,	0,	0,	0x2198,	/* SOUTH_EAST_ARROW */
  GDK_Multi_key,	GDK_KP_Down,	GDK_space,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
  GDK_Multi_key,	GDK_KP_Down,	GDK_2,	0,	0,	0x21CA,	/* DOWNWARDS_PAIRED_ARROWS */
  GDK_Multi_key,	GDK_KP_Down,	GDK_equal,	0,	0,	0x21D3,	/* DOWNWARDS_DOUBLE_ARROW */
  GDK_Multi_key,	GDK_KP_Down,	GDK_KP_Left,	0,	0,	0x2199,	/* SOUTH_WEST_ARROW */
  GDK_Multi_key,	GDK_KP_Down,	GDK_KP_Up,	0,	0,	0x2195,	/* UP_DOWN_ARROW */
  GDK_Multi_key,	GDK_KP_Down,	GDK_KP_Right,	0,	0,	0x2198,	/* SOUTH_EAST_ARROW */
  GDK_Multi_key,	GDK_KP_Down,	GDK_KP_Down,	0,	0,	0x2193,	/* DOWNWARDS_ARROW */
};

static const GtkComposeTable quimby_compose_table = {
  quimby_compose_seqs,
  4,
  G_N_ELEMENTS (quimby_compose_seqs) / 6
};

static void     quimby_class_init         (GtkIMQuimbyContextClass  *class);
static void     quimby_init               (GtkIMQuimbyContext       *im_context_simple);

static GtkIMContextSimpleClass *parent_class = NULL;

static GType type_quimby = 0;

static const GtkIMContextInfo quimby_info = 
{
  "quimby",     /* ID */
  N_("Quimby"), /* Human readable name */
  GETTEXT_PACKAGE, /* Translation domain */
  QUIMBY_LOCALEDIR,       /* Dir for bindtextdomain */
  "br:ca:ch:cs:cy:da:de:en:eo:es:et:eu:fi:fo:fr:fy:ga:gd:gl:hr:hu:id:is:it:kl:lb:lt:lv:mi:mt:nl:no:oc:pl:pt:rm:sk:sl:sq:sv:tk:tl:tn",            /* Languages for which this module is the default */  
};

void
gtk_quimby_im_context_register_type (GTypeModule *module)
{
    if (type_quimby == 0)
    {
	static const GTypeInfo object_info =
	    {
		sizeof (GtkIMQuimbyContextClass),
		(GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) quimby_class_init,
		NULL,           /* class_finalize */
		NULL,           /* class_data */
		sizeof (GtkIMQuimbyContext),
		0,              /* n_preallocs */
		(GInstanceInitFunc) quimby_init,
		0,
	    };
	type_quimby = 
	    g_type_module_register_type (module,
					 GTK_TYPE_IM_CONTEXT_SIMPLE,
					 "GtkIMContextSimpleQuimby",
					 &object_info, 0);
    }
}

GType gtk_quimby_im_context_get_type(void)
{
  g_assert(type_quimby != 0);
  return type_quimby;
}


static void clipboard_request_test_cb (GtkClipboard *clipboard,
					const gchar *text,
					gpointer data)
{
  GtkIMContext *context = (GtkIMContext *) data;
  if (text)
    g_signal_emit_by_name (context, "commit", text);
}

static gboolean
quimby_filter_keypress (GtkIMContext *context,
				       GdkEventKey  *event)
{
    GtkIMContextSimple *context_simple = GTK_IM_CONTEXT_SIMPLE (context);
    GtkIMQuimbyContext *context_quimby = GTK_QUIMBY_IM_CONTEXT (context);
    gunichar toggled = 0;
    gchar *text;
    gint cursor_index;
    gint len;
    gchar utf8_buf[10];
    /* Transpose characters */
    if (event->keyval == GDK_F22 && event->type == GDK_KEY_PRESS)
    {
	if (gtk_im_context_get_surrounding(GTK_IM_CONTEXT (context), &text, &cursor_index))
	{
	    /* note: as is, this doesn't check for combining marks, other special unicode characters */
	    if (g_utf8_strlen(text, -1) < cursor_index + 1)
	    {
		gunichar second = g_utf8_get_char(text+cursor_index+1);
		if (second)
		{
		    gtk_im_context_delete_surrounding (GTK_IM_CONTEXT (context), 1, 1);
		    len = g_unichar_to_utf8(second, utf8_buf);
		    utf8_buf[len] = '\0';
		    g_signal_emit_by_name (context, "commit", &utf8_buf);
		}
	    }
	}
    }
    /* Toggle capitalization */
    if (event->keyval == GDK_F23 && event->type == GDK_KEY_PRESS)
    {
	if (gtk_im_context_get_surrounding(GTK_IM_CONTEXT (context), &text, &cursor_index))
	{
	    gunichar chr = g_utf8_get_char(text+cursor_index);
	    if (g_unichar_islower(chr))
	    {
		toggled = g_unichar_toupper(chr);
	    }
	    else 
	    {
		if (g_unichar_isupper(chr))
		{
		    toggled = g_unichar_tolower(chr);
		}
		else
		{
		    toggled = chr;
		}
	    }
	    gtk_im_context_delete_surrounding (GTK_IM_CONTEXT (context), 0, 1);
	    len = g_unichar_to_utf8(toggled, utf8_buf);
	    utf8_buf[len] = '\0';
	    g_signal_emit_by_name (context, "commit", &utf8_buf);
	}
    } 
    if ((event->keyval == GDK_Insert && context_simple->compose_buffer[0] == GDK_Multi_key) ||
	(event->keyval == GDK_F24))
    {
	if (event->type == GDK_KEY_PRESS)
	{
	    return TRUE;
	}
	else
	{
	    gtk_clipboard_request_text (gtk_clipboard_get(GDK_SELECTION_PRIMARY),
					clipboard_request_test_cb,
					context);
	    context_simple->compose_buffer[0] = 0;
	    
	}
    }

    if (!context_quimby->chord_state)
    {
    	if (event->type == GDK_KEY_PRESS && 
	    (event->keyval != context_quimby->last_keyval || context_quimby->last_event_type != GDK_KEY_PRESS))
	{
	    if (gtk_im_context_get_surrounding(GTK_IM_CONTEXT (context), &text, &cursor_index))
	    {
		StartPositionType startpos = start_position(text, cursor_index);
		if (startpos != NOT_START)
		{
		    if (startpos == SENTENCE_START)
		    {
			context_quimby->chord_capital = TRUE;
		    }
		    context_quimby->chord_state = TRUE;
		    context_quimby->num_keys_down++;
		    add_to_chord (context_quimby, event->keyval);
		}
	    }
	}
    }
    else
    {
	if (event->type == GDK_KEY_PRESS && 
	    (event->keyval != context_quimby->last_keyval || context_quimby->last_event_type != GDK_KEY_PRESS))
	{
	    context_quimby->num_keys_down++;
	    add_to_chord (context_quimby, event->keyval);
	}
	if (event->type == GDK_KEY_RELEASE)
	{
	    context_quimby->num_keys_down--;
	    if (context_quimby->num_keys_down == 0)
	    {
		text = chord_lookup(context_quimby);
		if (text)
		{
		    gtk_im_context_delete_surrounding (GTK_IM_CONTEXT (context), -context_quimby->chord_length - 1, context_quimby->chord_length + 1);
		    if (context_quimby->chord_capital)
		    {
			guint pos;
			gchar first_char_utf8_buf[20];
			gunichar first_char = g_utf8_get_char (text);
			first_char = g_unichar_toupper (first_char);
			pos = g_unichar_to_utf8 (first_char, first_char_utf8_buf);
			first_char_utf8_buf[pos] = 0;
			g_signal_emit_by_name (context, "commit", first_char_utf8_buf);
			text = g_utf8_next_char(text);
		    }
		    g_signal_emit_by_name (context, "commit", text);
		}
		clear_chord (context_quimby);
	    }
	}
    }
    context_quimby->last_event_type = event->type;
    context_quimby->last_keyval = event->keyval;
    return ((GtkIMContextClass *)parent_class)->filter_keypress (context, event);
}

static void
quimby_class_init (GtkIMQuimbyContextClass *klass)
{
    parent_class = GTK_IM_CONTEXT_SIMPLE_CLASS (g_type_class_peek (g_type_parent (type_quimby)));
    ((GtkIMContextClass *)klass)->filter_keypress = quimby_filter_keypress;
}

void 
im_module_exit ()
{

}

static void
quimby_init (GtkIMQuimbyContext *im_context)
{
    gtk_im_context_simple_add_table (GTK_IM_CONTEXT_SIMPLE (im_context),
				     quimby_compose_seqs,
				     4,
				     G_N_ELEMENTS (quimby_compose_seqs) / 6);
    load_dictionary (im_context);
    im_context->chord_length = 0;
    im_context->num_keys_down = 0;
    im_context->chord_capital = FALSE;
    im_context->chord_state = FALSE;
    im_context->space_in_chord = FALSE;
    im_context->last_event_type = GDK_NOTHING;
    
}


static const GtkIMContextInfo *info_list[] = 
{
    &quimby_info,
};


void
im_module_init (GTypeModule *module)
{
    gtk_quimby_im_context_register_type (module);
}


void 
im_module_list (const GtkIMContextInfo ***contexts, gint *n_contexts)
{
    *contexts = info_list;
    *n_contexts = G_N_ELEMENTS (info_list);
}


GtkIMContextSimple *
im_module_create (const gchar *context_id)
{
    if (strcmp (context_id, "quimby") == 0)
    {
	GtkIMContext* imcontext = GTK_IM_CONTEXT (g_object_new (GTK_TYPE_QUIMBY_IM_CONTEXT, NULL));
	return imcontext;
    }
    else
	return NULL;
}
