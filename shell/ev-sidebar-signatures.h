/* ev-sidebar-signatures.h
 *  this file is part of evince, a gnome document viewer
 *
 * Copyright (C) 2012 Vasco Dias  <contact@vascodias.me>
 *
 * Evince is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Evince is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __EV_SIDEBAR_SIGNATURES_H__
#define __EV_SIDEBAR_SIGNATURES_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _EvSidebarSignatures        EvSidebarSignatures;
typedef struct _EvSidebarSignaturesClass   EvSidebarSignaturesClass;
typedef struct _EvSidebarSignaturesPrivate EvSidebarSignaturesPrivate;

#define EV_TYPE_SIDEBAR_SIGNATURES              (ev_sidebar_signatures_get_type())
#define EV_SIDEBAR_SIGNATURES(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), EV_TYPE_SIDEBAR_SIGNATURES, EvSidebarSignatures))
#define EV_SIDEBAR_SIGNATURES_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), EV_TYPE_SIDEBAR_SIGNATURES, EvSidebarSignaturesClass))
#define EV_IS_SIDEBAR_SIGNATURES(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), EV_TYPE_SIDEBAR_SIGNATURES))
#define EV_IS_SIDEBAR_SIGNATURES_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), EV_TYPE_SIDEBAR_SIGNATURES))
#define EV_SIDEBAR_SIGNATURES_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), EV_TYPE_SIDEBAR_SIGNATURES, EvSidebarSignaturesClass))

struct _EvSidebarSignatures {
  GtkVBox base_instance;

  EvSidebarSignaturesPrivate *priv;
};

struct _EvSidebarSignaturesClass {
  GtkVBoxClass base_class;

  /* Signals */
};

GType      ev_sidebar_signatures_get_type     (void) G_GNUC_CONST;
GtkWidget *ev_sidebar_signatures_new          (void);

G_END_DECLS

#endif /* __EV_SIDEBAR_SIGNATURES_H__ */
