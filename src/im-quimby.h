/*
 * Copyright (C) 2014 Alexandre Muniz
 * 
 * Author: Alexandre Muniz <pz@puzzlezapper.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __IM_QUIMBY_H__
#define __IM_QUIMBY_H__

#include <gtk/gtkimcontext.h>
#include <gtk/gtkimcontextsimple.h>

G_BEGIN_DECLS

#define GTK_TYPE_QUIMBY_IM_CONTEXT              (gtk_quimby_im_context_get_type())
#define GTK_QUIMBY_IM_CONTEXT(obj)              (GTK_CHECK_CAST ((obj), GTK_TYPE_QUIMBY_IM_CONTEXT, GtkIMQuimbyContext))
#define GTK_QUIMBY_IM_CONTEXT_CLASS(klass)      (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_QUIMBY_IM_CONTEXT, GtkIMQuimbyContextClass))
#define GTK_IS_QUIMBY_IM_CONTEXT(obj)           (GTK_CHECK_TYPE ((obj), GTK_TYPE_QUIMBY_IM_CONTEXT))
#define GTK_IS_QUIMBY_IM_CONTEXT_CLASS(klass)   (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_QUIMBY_IM_CONTEXT))
#define GTK_QUIMBY_IM_CONTEXT_GET_CLASS(obj)    (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_QUIMBY_IM_CONTEXT, GtkIMQuimbyContextClass)

#define MAX_CHORD_CHARS 6

typedef struct _GtkIMQuimbyContext GtkIMQuimbyContext;

struct _GtkIMQuimbyContext
{
    GtkIMContextSimple parent;
    GHashTable *chord_dict;
    guint chord_chars[MAX_CHORD_CHARS];
    guint chord_length;
    guint num_keys_down;
    gboolean chord_capital;
    gboolean chord_state;
    gboolean space_in_chord;
    GdkEventType last_event_type;
    guint last_keyval;
//    GtkWidget *widget;
};


typedef struct _GtkIMQuimbyContextClass  GtkIMQuimbyContextClass;

struct _GtkIMQuimbyContextClass
{
  GtkIMContextSimpleClass parent_class;
};

void gtk_quimby_im_context_register_type(GTypeModule* type_module);
GType gtk_quimby_im_context_get_type (void) G_GNUC_CONST;
GtkIMContext *gtk_quimby_im_context_new (void);

G_END_DECLS

#endif
