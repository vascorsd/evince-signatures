/* ev-signatures.h
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

#if !defined (__EV_EVINCE_DOCUMENT_H_INSIDE__) && !defined (EVINCE_COMPILATION)
#error "Only <evince-document.h> can be included directly."
#endif

#ifndef __EV_SIGNATURE_H__
#define __EV_SIGNATURE_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _EvSignature        EvSignature;
typedef struct _EvSignatureClass   EvSignatureClass;
typedef struct _EvSignaturePrivate EvSignaturePrivate;

#define EV_TYPE_SIGNATURE              (ev_signature_get_type())
#define EV_SIGNATURE(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), EV_TYPE_SIGNATURE, EvSignature))
#define EV_SIGNATURE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), EV_TYPE_SIGNATURE, EvSignatureClass))
#define EV_IS_SIGNATURE(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), EV_TYPE_SIGNATURE))
#define EV_IS_SIGNATURE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), EV_TYPE_SIGNATURE))
#define EV_SIGNATURE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), EV_TYPE_SIGNATURE, EvSignatureClass))


struct _EvSignature {
	GObject base_instance;
	
	EvSignaturePrivate *priv;
};

struct _EvSignatureClass {
	GObjectClass base_class;
};

GType         ev_signature_get_type                (void) G_GNUC_CONST;
EvSignature  *ev_signature_new                     (const gchar  *signer_name,
                                                    gboolean     is_document_valid,
                                                    gboolean     is_identity_known,
                                                    const gchar  *signature_time);

G_END_DECLS

#endif /* __EV_SIGNATURE_H__ */
