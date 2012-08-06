/* ev-sidebar-signatures.c
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

#include "config.h"

#include <glib/gi18n.h>             // ----- support for translation

#include "ev-sidebar-signatures.h"  // ----- my header file
#include "ev-sidebar-page.h"        // ----- needed for all the Ev* objects and stuff

#include "ev-document-signatures.h" // ----- implement the document interface

struct _EvSidebarSignaturesPrivate {
  GtkWidget *swindow;
  GtkWidget *tree_view;

};

enum {
  PROP_0,
  PROP_WIDGET
};

enum {
  COL_SIGN_INFO,
  N_COLUMNS
};

// ------------------------------------------------------ define functions related to the interface binding
static void ev_sidebar_signatures_page_iface_init      (EvSidebarPageInterface    *iface);
static gboolean ev_sidebar_signatures_support_document (EvSidebarPage             *sidebar_page,
                                                        EvDocument                *document);
static void ev_sidebar_signatures_set_model            (EvSidebarPage             *sidebar_page,
                                                        EvDocumentModel           *model);
static const gchar* ev_sidebar_signatures_get_label    (EvSidebarPage             *sidebar_page);

// --------------------------------------------------------- define functions related to object construction
GtkWidget* ev_sidebar_signatures_new (void);
static void ev_sidebar_signatures_init                 (EvSidebarSignatures       *ev_sign);
static void ev_sidebar_signatures_class_init           (EvSidebarSignaturesClass  *ev_sidebar_signatures_class);
static void ev_sidebar_signatures_get_property         (GObject                   *object,
                                                        guint                     prop_id,
                                                        GValue                    *value,
                                                        GParamSpec                *pspec);
static void ev_sidebar_signatures_dispose              (GObject                   *object);

// -------------------------------------------------------------------------------------- define my functions
static GtkWidget* ev_sidebar_signatures_construct_tree_view (void);

static GtkTreeModel* ev_sidebar_signatures_tree_create_load_model (void);

static void ev_sidebar_signatures_tree_sign_info       (GtkTreeStore *store,
                                                        GtkTreeIter  *iter);

// --------------------------------------------------------------- set this object to implement the interface
G_DEFINE_TYPE_EXTENDED (EvSidebarSignatures,
                        ev_sidebar_signatures,
                        GTK_TYPE_VBOX,
                        0,
                        G_IMPLEMENT_INTERFACE (EV_TYPE_SIDEBAR_PAGE,
                        ev_sidebar_signatures_page_iface_init))

#define EV_SIDEBAR_SIGNATURES_GET_PRIVATE(object) \
                (G_TYPE_INSTANCE_GET_PRIVATE ((object), EV_TYPE_SIDEBAR_SIGNATURES, EvSidebarSignaturesPrivate))

// -------------------------------------------------------------------- bind the functions to the interface
static void
ev_sidebar_signatures_page_iface_init (EvSidebarPageInterface *iface)
{
  iface->support_document = ev_sidebar_signatures_support_document;
  iface->set_model = ev_sidebar_signatures_set_model;
  iface->get_label = ev_sidebar_signatures_get_label;
}

// ----------------------------------------------------------------- functions that implement the interface
static gboolean
ev_sidebar_signatures_support_document (EvSidebarPage   *sidebar_page,
                                        EvDocument      *document)
{
  g_print("ev-sidebar-signatures::ev_sidebar_signatures_support_document, called!\n");
  return (EV_IS_DOCUMENT_SIGNATURES (document) &&
          ev_document_signatures_has_signatures (EV_DOCUMENT_SIGNATURES (document)));
}

static const gchar *
ev_sidebar_signatures_get_label (EvSidebarPage *sidebar_page)
{
  g_print("ev-sidebar-signatures::ev_sidebar_signatures_get_label, called!\n");
  return _("Signatures");
}

static void
ev_sidebar_signatures_set_model (EvSidebarPage   *sidebar_page,
                                 EvDocumentModel *model)
{
  g_print("ev-sidebar-signatures::ev_sidebar_signatures_set_model, called!\n");
}

// ----------------------------------------------------------------------------------- Object construction
GtkWidget *
ev_sidebar_signatures_new (void)
{
  return GTK_WIDGET (g_object_new (EV_TYPE_SIDEBAR_SIGNATURES, NULL));
}

// Don't understand if this i needed or even waht it is.
// I think it's related to knowing it the widget is being shown or not.
//static void
//ev_sidebar_signatures_map (gtkwidget *widget)
//{
//	evsidebarsignatures *sidebar;
//
//	sidebar = ev_sidebar_signatures (widget);
//
//	gtk_widget_class (ev_sidebar_signatures_parent_class)->map (widget);
//}

static void
ev_sidebar_signatures_init (EvSidebarSignatures *ev_sign)
{
  EvSidebarSignaturesPrivate *priv;

  // initialize the private structure that holds everything we need
  priv = ev_sign->priv = EV_SIDEBAR_SIGNATURES_GET_PRIVATE (ev_sign);

  // create a scrolled window for the sidebar
  priv->swindow = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (priv->swindow),
                                       GTK_SHADOW_IN);

  // the main widget of the sidebar will be the scrollable area
  gtk_box_pack_start (GTK_BOX (ev_sign), priv->swindow, TRUE, TRUE, 0);

  // create the tree view where all the info is contained
  priv->tree_view = ev_sidebar_signatures_construct_tree_view ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree_view), FALSE);

  // add the tree view to the scrolled area
  gtk_container_add (GTK_CONTAINER (priv->swindow), priv->tree_view);

  gtk_widget_show_all (GTK_WIDGET (ev_sign));
}

static void
ev_sidebar_signatures_class_init (EvSidebarSignaturesClass *ev_sidebar_signatures_class)
{
  GObjectClass *g_object_class;
  //GtkWidgetClass *gtk_widget_class;

  g_object_class = G_OBJECT_CLASS (ev_sidebar_signatures_class);
  //gtk_widget_class = GTK_WIDGET_CLASS (ev_sidebar_signatures_class);

  g_object_class->get_property = ev_sidebar_signatures_get_property;
  g_object_class->dispose = ev_sidebar_signatures_dispose;
  //gtk_widget_class->map = ev_sidebar_signatures_map;

  g_object_class_override_property (g_object_class, PROP_WIDGET, "main-widget");

  g_type_class_add_private (g_object_class, sizeof (EvSidebarSignaturesPrivate));
}

static void
ev_sidebar_signatures_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  EvSidebarSignatures *sidebar = EV_SIDEBAR_SIGNATURES (object);

  switch (prop_id)
    {
    case PROP_WIDGET:
      g_value_set_object (value, sidebar->priv->tree_view);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
ev_sidebar_signatures_dispose (GObject *object)
{
  //EvSidebarSignatures *sidebar = EV_SIDEBAR_SIGNATURES (object);

  // we should clean the stuff we use. understand this better, unrefs and etc
}

// ----------------------------------------------------------------------------------- And my functions
static GtkWidget *
ev_sidebar_signatures_construct_tree_view ()
{
  // construct everything related to the tree view
  GtkWidget *tree_view = gtk_tree_view_new ();
  GtkTreeViewColumn *col = gtk_tree_view_column_new ();
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
  GtkTreeModel *model = ev_sidebar_signatures_tree_create_load_model ();

  // make the associations for it to show something
  gtk_tree_view_column_set_title (col, "Signatures");
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), col);

  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_SIGN_INFO);

  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);

  return tree_view;
}

static GtkTreeModel *
ev_sidebar_signatures_tree_create_load_model ()
{
  GtkTreeStore *store = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING);
  GtkTreeIter iter;

  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter, COL_SIGN_INFO, "Signature ONE  ----------------------------------------------------", -1);
  ev_sidebar_signatures_tree_sign_info (store, &iter);

  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter, COL_SIGN_INFO, "Signature TWO  ---", -1);
  ev_sidebar_signatures_tree_sign_info (store, &iter);

  return GTK_TREE_MODEL (store);
}

static void
ev_sidebar_signatures_tree_sign_info (GtkTreeStore *store,
                                                 GtkTreeIter  *iter)
{
  GtkTreeIter child;

  gtk_tree_store_append (store, &child, iter);
  gtk_tree_store_set (store, &child, COL_SIGN_INFO, "Document ----", -1);
  gtk_tree_store_append (store, &child, iter);
  gtk_tree_store_set (store, &child, COL_SIGN_INFO, "Indentity ----", -1);
  gtk_tree_store_append (store, &child, iter);
  gtk_tree_store_set (store, &child, COL_SIGN_INFO, "Date/Time ----", -1);
}
