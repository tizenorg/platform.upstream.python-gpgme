/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
    pygpgme - a Python wrapper for the gpgme library
    Copyright (C) 2006  James Henstridge

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "pygpgme.h"
#include <structmember.h>

static void
pygpgme_newsig_dealloc(PyGpgmeNewSignature *self)
{
    Py_XDECREF(self->type);
    Py_XDECREF(self->pubkey_algo);
    Py_XDECREF(self->hash_algo);
    Py_XDECREF(self->timestamp);
    Py_XDECREF(self->fpr);
    Py_XDECREF(self->sig_class);
    PyObject_Del(self);
}

static PyMemberDef pygpgme_newsig_members[] = {
    { "type", T_OBJECT, offsetof(PyGpgmeNewSignature, type), RO},
    { "pubkey_algo", T_OBJECT, offsetof(PyGpgmeNewSignature, pubkey_algo), RO},
    { "hash_algo", T_OBJECT, offsetof(PyGpgmeNewSignature, hash_algo), RO},
    { "timestamp", T_OBJECT, offsetof(PyGpgmeNewSignature, timestamp), RO},
    { "fpr", T_OBJECT, offsetof(PyGpgmeNewSignature, fpr), RO},
    { "sig_class", T_OBJECT, offsetof(PyGpgmeNewSignature, sig_class), RO},
    { NULL, 0, 0, 0}
};

PyTypeObject PyGpgmeNewSignature_Type = {
    PyObject_HEAD_INIT(NULL)
    0,
    "gpgme.NewSignature",
    sizeof(PyGpgmeNewSignature),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_init = pygpgme_no_constructor,
    .tp_dealloc = (destructor)pygpgme_newsig_dealloc,
    .tp_members = pygpgme_newsig_members,
};

PyObject *
pygpgme_newsiglist_new(gpgme_new_signature_t siglist)
{
    PyObject *list;
    gpgme_new_signature_t sig;

    list = PyList_New(0);
    for (sig = siglist; sig != NULL; sig = sig->next) {
        PyGpgmeNewSignature *item = PyObject_New(PyGpgmeNewSignature,
                                                 &PyGpgmeNewSignature_Type);
        if (item == NULL) {
            Py_DECREF(list);
            return NULL;
        }
        item->type = PyInt_FromLong(sig->type);
        item->pubkey_algo = PyInt_FromLong(sig->pubkey_algo);
        item->hash_algo = PyInt_FromLong(sig->hash_algo);
        item->timestamp = PyInt_FromLong(sig->timestamp);
        if (sig->fpr) {
            item->fpr = PyString_FromString(sig->fpr);
        } else {
            Py_INCREF(Py_None);
            item->fpr = Py_None;
        }
        item->sig_class = PyInt_FromLong(sig->sig_class);
        if (PyErr_Occurred()) {
            Py_DECREF(item);
            Py_DECREF(list);
            return NULL;
        }
        PyList_Append(list, (PyObject *)item);
        Py_DECREF(item);
    }
    return list;
}

static void
pygpgme_sig_dealloc(PyGpgmeSignature *self)
{
    Py_XDECREF(self->summary);
    Py_XDECREF(self->fpr);
    Py_XDECREF(self->status);
    Py_XDECREF(self->notations);
    Py_XDECREF(self->timestamp);
    Py_XDECREF(self->exp_timestamp);
    Py_XDECREF(self->wrong_key_usage);
    Py_XDECREF(self->validity);
    Py_XDECREF(self->validity_reason);
    PyObject_Del(self);
}

static PyMemberDef pygpgme_sig_members[] = {
    { "summary", T_OBJECT, offsetof(PyGpgmeSignature, summary), RO},
    { "fpr", T_OBJECT, offsetof(PyGpgmeSignature, fpr), RO},
    { "status", T_OBJECT, offsetof(PyGpgmeSignature, status), RO},
    { "notations", T_OBJECT, offsetof(PyGpgmeSignature, notations), RO},
    { "timestamp", T_OBJECT, offsetof(PyGpgmeSignature, timestamp), RO},
    { "exp_timestamp", T_OBJECT,
      offsetof(PyGpgmeSignature, exp_timestamp), RO},
    { "wrong_key_usage", T_OBJECT,
      offsetof(PyGpgmeSignature, wrong_key_usage), RO},
    { "validity", T_OBJECT, offsetof(PyGpgmeSignature, validity), RO},
    { "validity_reason", T_OBJECT,
      offsetof(PyGpgmeSignature, validity_reason), RO},
    { NULL, 0, 0, 0}
};

PyTypeObject PyGpgmeSignature_Type = {
    PyObject_HEAD_INIT(NULL)
    0,
    "gpgme.Signature",
    sizeof(PyGpgmeSignature),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_init = pygpgme_no_constructor,
    .tp_dealloc = (destructor)pygpgme_sig_dealloc,
    .tp_members = pygpgme_sig_members,
};

PyObject *
pygpgme_siglist_new(gpgme_signature_t siglist)
{
    PyObject *list;
    gpgme_signature_t sig;
    gpgme_sig_notation_t not;

    list = PyList_New(0);
    for (sig = siglist; sig != NULL; sig = sig->next) {
        PyGpgmeSignature *item = PyObject_New(PyGpgmeSignature,
                                              &PyGpgmeSignature_Type);
        if (item == NULL) {
            Py_DECREF(list);
            return NULL;
        }
        item->summary = PyInt_FromLong(sig->summary);
        if (sig->fpr) {
            item->fpr = PyString_FromString(sig->fpr);
        } else {
            Py_INCREF(Py_None);
            item->fpr = Py_None;
        }
        item->status = pygpgme_error_object(sig->status);
        item->notations = PyList_New(0);
        for (not = sig->notations; not != NULL; not = not->next) {
            PyObject *pynot = Py_BuildValue("(zz)", not->name, not->value);

            if (!pynot)
                break;
            PyList_Append(item->notations, pynot);
            Py_DECREF(pynot);
        }
        item->timestamp = PyInt_FromLong(sig->timestamp);
        item->exp_timestamp = PyInt_FromLong(sig->exp_timestamp);
        item->wrong_key_usage = PyBool_FromLong(sig->wrong_key_usage);
        item->validity = PyInt_FromLong(sig->validity);
        item->validity_reason = pygpgme_error_object(sig->validity_reason);
        if (PyErr_Occurred()) {
            Py_DECREF(item);
            Py_DECREF(list);
            return NULL;
        }
        PyList_Append(list, (PyObject *)item);
        Py_DECREF(item);
    }
    return list;
}
