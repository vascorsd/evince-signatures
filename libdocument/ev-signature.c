/* ev-signature.c
 *  this file is part of evince, a gnome document viewer
 *
 * Copyright © 2012 Vasco Dias  <contact@vascodias.me>
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

#include <gtk/gtk.h>

#include "ev-signature.h"

enum {
  PROP_0,
  PROP_SIGNER_NAME,
  PROP_SIGN_VALID,
  PROP_SIGNER_KNOWN,
  PROP_SIGN_TIME
};

struct _EvSignaturePrivate {
  gchar     *signer_name;
  gboolean   signature_valid;
  gboolean   signer_identity_known;
  gchar     *signature_time;
};

#define EV_SIGNATURE_GET_PRIVATE(object) \
                (G_TYPE_INSTANCE_GET_PRIVATE ((object), EV_TYPE_SIGNATURE, EvSignaturePrivate))

G_DEFINE_TYPE (EvSignature, ev_signature, G_TYPE_OBJECT)

static void
ev_signature_finalize (GObject *object)
{
  EvSignature *signature = EV_SIGNATURE (object);

  g_free (signature->priv->signer_name);
  signature->priv->signer_name = NULL;

  g_free (signature->priv->signature_time);
  signature->priv->signature_time = NULL;

  G_OBJECT_CLASS (ev_signature_parent_class)->finalize (object);
}

static void
ev_signature_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *param_spec)
{
  EvSignature *signature = EV_SIGNATURE (object);

  switch (prop_id)
    {
    case PROP_SIGNER_NAME:
      signature->priv->signer_name = g_value_dup_string (value);
      break;

    case PROP_SIGN_VALID:
      signature->priv->signature_valid = g_value_get_boolean (value);
      break;

    case PROP_SIGNER_KNOWN:
      signature->priv->signer_identity_known = g_value_get_boolean (value);
      break;

    case PROP_SIGN_TIME:
      signature->priv->signature_time = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
                                         prop_id,
                                         param_spec);
      break;
    }
}

static void
ev_signature_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  EvSignature *signature = EV_SIGNATURE (object);

  switch (property_id)
    {
    case PROP_SIGNER_NAME:
      g_value_set_string (value, signature->priv->signer_name);
      break;

    case PROP_SIGN_VALID:
      g_value_set_boolean (value, signature->priv->signature_valid);
      break;

    case PROP_SIGNER_KNOWN:
      g_value_set_boolean (value, signature->priv->signer_identity_known);
      break;

    case PROP_SIGN_TIME:
      g_value_set_string (value, signature->priv->signature_time);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ev_signature_class_init (EvSignatureClass *klass)
{
  GObjectClass *g_object_class = G_OBJECT_CLASS (klass);

  g_object_class->set_property = ev_signature_set_property;
  g_object_class->get_property = ev_signature_get_property;

  g_type_class_add_private (g_object_class, sizeof (EvSignaturePrivate));

  /* Properties */
  g_object_class_install_property (g_object_class,
                                   PROP_SIGNER_NAME,
                                   g_param_spec_string ("signer-name",
                                                        "Name",
                                                        "The name of the entity that signed",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (g_object_class,
                                   PROP_SIGN_VALID,
                                   g_param_spec_boolean ("signature-valid",
                                                        "SignatureValid",
                                                        "Is this signature considered valid or not",
                                                        FALSE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (g_object_class,
                                   PROP_SIGNER_KNOWN,
                                   g_param_spec_boolean ("signer-identity-known",
                                                        "SignerIdentityKnown",
                                                        "Is the signer that created this signature known or not",
                                                        FALSE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (g_object_class,
                                   PROP_SIGN_TIME,
                                   g_param_spec_string ("signature-time",
                                                       "SignatureTime",
                                                       "The time associated with the signature",
                                                       NULL,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY |
                                                       G_PARAM_STATIC_STRINGS));

  g_object_class->finalize = ev_signature_finalize;
}

static void
ev_signature_init (EvSignature *signature)
{
  signature->priv = EV_SIGNATURE_GET_PRIVATE (signature);

  signature->priv->signer_name = NULL;
  signature->priv->signature_time = NULL;
}

EvSignature *
ev_signature_new (const gchar  *signer_name,
                  gboolean      signature_valid,
                  gboolean      signer_identity_known,
                  const gchar  *signature_time)
{
  EvSignature *signature = g_object_new (EV_TYPE_SIGNATURE,
                                         "signer-name", signer_name,
                                         "signature-valid", signature_valid,
                                         "signer-identity-known", signer_identity_known,
                                         "signature-time", signature_time,
                                         NULL);

  return signature;
}
