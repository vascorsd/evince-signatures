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
#include "ev-jobs.h"                // ----- this one is needed for the threads
#include "ev-job-scheduler.h"       // ----- and with this we can change threads priority

struct _EvSidebarSignaturesPrivate {
  GtkWidget    *swindow;
  GtkWidget    *tree_view;

  GtkTreeStore *model;
};

enum {
  PROP_0,
  PROP_WIDGET
};

enum {
  COL_ICON,
  COL_HAS_ICON,
  COL_SIGN_TEXT,
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

static void ev_sidebar_signatures_tree_add_sign_info   (GtkTreeStore               *model,
                                                        gchar                      *signer_name,
                                                        gboolean                    is_valid_do,
                                                        gboolean                    is_id_known,
                                                        gchar                      *sign_time);


static void job_finished_callback                      (EvJobSignatures            *job,
                                                        EvSidebarSignatures        *sidebar);
static void ev_sidebar_signatures_document_changed_cb  (EvDocumentModel            *model,
                                                        GParamSpec                 *pspec,
                                                        EvSidebarSignatures        *sidebar_sign);

static void render_icon_func                           (GtkTreeViewColumn          *column,
                                                        GtkCellRenderer            *renderer,
                                                        GtkTreeModel               *model,
                                                        GtkTreeIter                *iter,
                                                        gpointer                    user_data);

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
  return (EV_IS_DOCUMENT_SIGNATURES (document) &&
          ev_document_signatures_has_signatures (EV_DOCUMENT_SIGNATURES (document)));
}

static const gchar *
ev_sidebar_signatures_get_label (EvSidebarPage *sidebar_page)
{
  return _("Signatures");
}

static void
job_finished_callback (EvJobSignatures *job, EvSidebarSignatures *sidebar)
{
  GList *l;
  GtkTreeStore *model = sidebar->priv->model;

  gchar *signer_name;
  gboolean is_doc_valid;
  gboolean is_id_known;
  gchar *sign_time;

	for (l = job->signatures; l && l->data; l = g_list_next (l)) {
    EvSignature *signature = EV_SIGNATURE (l->data);
    
    g_object_get (G_OBJECT (signature),
                  "signer-name", &signer_name,
                  "is-doc-valid", &is_doc_valid,
                  "is-id-known", &is_id_known,
                  "sign-time", &sign_time,
                  NULL);

    ev_sidebar_signatures_tree_add_sign_info (model, signer_name, is_doc_valid, is_id_known, sign_time);
  }
}

static void
ev_sidebar_signatures_document_changed_cb (EvDocumentModel     *model,
                                           GParamSpec          *pspec,
                                           EvSidebarSignatures *sidebar_sign)
{
  EvDocument *document = ev_document_model_get_document (model);
  EvJob *job = ev_job_signatures_new (document);
 
  if (!EV_IS_DOCUMENT_SIGNATURES (document))
    return;

  if (!ev_document_signatures_has_signatures (EV_DOCUMENT_SIGNATURES (document)))
    return;

  g_signal_connect (job, "finished",
			  G_CALLBACK (job_finished_callback),
			  sidebar_sign);
  
  /* The priority doesn't matter for this job */
	ev_job_scheduler_push_job (job, EV_JOB_PRIORITY_NONE);
}

static void
ev_sidebar_signatures_set_model (EvSidebarPage   *sidebar_page,
                                 EvDocumentModel *model)
{
  g_signal_connect (model, "notify::document",
        G_CALLBACK (ev_sidebar_signatures_document_changed_cb),
			  sidebar_page);
}

// ----------------------------------------------------------------------------------- Object construction
GtkWidget *
ev_sidebar_signatures_new (void)
{
  return GTK_WIDGET (g_object_new (EV_TYPE_SIDEBAR_SIGNATURES, NULL));
}

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
  priv->tree_view = gtk_tree_view_new ();
  GtkTreeViewColumn *col = gtk_tree_view_column_new ();
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
  GtkCellRenderer *icon_renderer = gtk_cell_renderer_pixbuf_new (); 


  // create model
  priv->model = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING);

  // make the associations for it to show something
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree_view), col);

  gtk_tree_view_column_pack_start (col, icon_renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, icon_renderer, "stock-id", COL_ICON);
  gtk_tree_view_column_set_cell_data_func (col, icon_renderer, render_icon_func, NULL, NULL);

  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_SIGN_TEXT);
  
  // associate the model to the view
  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree_view), GTK_TREE_MODEL (priv->model));

  // some other options for the tree view
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree_view), FALSE);

  // add the tree view to the scrolled area
  gtk_container_add (GTK_CONTAINER (priv->swindow), priv->tree_view);

  gtk_widget_show_all (GTK_WIDGET (ev_sign));
}

static void
ev_sidebar_signatures_class_init (EvSidebarSignaturesClass *ev_sidebar_signatures_class)
{
  GObjectClass *g_object_class;

  g_object_class = G_OBJECT_CLASS (ev_sidebar_signatures_class);

  g_object_class->get_property = ev_sidebar_signatures_get_property;
  g_object_class->dispose = ev_sidebar_signatures_dispose;

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
  EvSidebarSignatures *sidebar = EV_SIDEBAR_SIGNATURES (object);
/*
  // we should clean the stuff we use. understand this better, unrefs and etc
  if (sidebar->priv->swindow) {
    g_object_unref (sidebar->priv->swindow);
    sidebar->priv->swindow = NULL;
  }

  if (sidebar->priv->tree_view) {
    g_object_unref (sidebar->priv->tree_view);
    sidebar->priv->tree_view = NULL;
  }

  if (sidebar->priv->model) {
    g_object_unref (sidebar->priv->model);
    sidebar->priv->model = NULL;
  }
*/
//  G_OBJECT_CLASS (ev_sidebar_signatures_parent_class)->dispose (object);
}

// ----------------------------------------------------------------------------------- And my functions
static void
render_icon_func (GtkTreeViewColumn *column,
                  GtkCellRenderer   *renderer,
                  GtkTreeModel      *model,
                  GtkTreeIter       *iter,
                  gpointer          user_data)
{
  gboolean show_icon;

  gtk_tree_model_get (model, iter, COL_HAS_ICON, &show_icon, -1);

  gtk_cell_renderer_set_visible (renderer, show_icon);
}

static void
ev_sidebar_signatures_tree_add_sign_info (GtkTreeStore  *model, 
                                          gchar         *signer_name,
                                          gboolean      is_valid_doc,
                                          gboolean      is_id_known,
                                          gchar         *sign_time)
{
  GtkTreeIter parent;
  GtkTreeIter child;

  // create the 1st level node with the signature name
  gtk_tree_store_append (model, &parent, NULL);
  // GTK_STOCK_STOP / GTK_STOCK_NO / GTK_STOCK_DIALOG_WARNING
  // choose icon based on valid info
  // ...
  gtk_tree_store_set (model, &parent, COL_SIGN_TEXT, signer_name, 
                                      COL_ICON, GTK_STOCK_OK,
                                      COL_HAS_ICON, TRUE,
                                      -1);

  // append the remaining information about the signature as child nodes
  gtk_tree_store_append (model, &child, &parent);
  const gchar *doc_valid = is_valid_doc ? _("Document is valid") : _("Document is invalid");
  gtk_tree_store_set (model, &child, COL_SIGN_TEXT, doc_valid, 
                                     COL_HAS_ICON, FALSE,
                                     -1);
  
  gtk_tree_store_append (model, &child, &parent);
  const gchar *id_known = is_id_known ? _("Signer is known") : _("Signer is unknown");
  gtk_tree_store_set (model, &child, COL_SIGN_TEXT, id_known, 
                                     COL_HAS_ICON, FALSE,
                                     -1);
  
  gtk_tree_store_append (model, &child, &parent);
  const gchar *time_text = sign_time ? sign_time : _("Time not available");
  gtk_tree_store_set (model, &child, COL_SIGN_TEXT, time_text, 
                                     COL_HAS_ICON, FALSE,
                                     -1);
}
