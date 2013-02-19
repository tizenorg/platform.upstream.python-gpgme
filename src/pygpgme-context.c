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

static gpgme_error_t
pygpgme_passphrase_cb(void *hook, const char *uid_hint,
                      const char *passphrase_info, int prev_was_bad,
                      int fd)
{
    PyObject *callback, *ret;
    PyGILState_STATE state;
    gpgme_error_t err;

    state = PyGILState_Ensure();
    callback = (PyObject *)hook;
    ret = PyObject_CallFunction(callback, "zzii", uid_hint, passphrase_info,
                                prev_was_bad, fd);
    err = pygpgme_check_pyerror();
    Py_XDECREF(ret);
    PyGILState_Release(state);
    return err;
}

static void
pygpgme_progress_cb(void *hook, const char *what, int type,
                    int current, int total)
{
    PyObject *callback, *ret;
    PyGILState_STATE state;

    state = PyGILState_Ensure();
    callback = (PyObject *)hook;
    ret = PyObject_CallFunction(callback, "ziii", what, type, current, total);
    PyErr_Clear();
    Py_XDECREF(ret);
    PyGILState_Release(state);
}

static void
pygpgme_context_dealloc(PyGpgmeContext *self)
{
    gpgme_passphrase_cb_t passphrase_cb;
    gpgme_progress_cb_t progress_cb;
    PyObject *callback;

    if (self->ctx) {
        /* free the passphrase callback */
        gpgme_get_passphrase_cb(self->ctx, &passphrase_cb, (void **)&callback);
        if (passphrase_cb == pygpgme_passphrase_cb) {
            Py_DECREF(callback);
        }

        /* free the progress callback */
        gpgme_get_progress_cb(self->ctx, &progress_cb, (void **)&callback);
        if (progress_cb == pygpgme_progress_cb) {
            Py_DECREF(callback);
        }

        gpgme_release(self->ctx);
    }
    self->ctx = NULL;
    PyObject_Del(self);
}

static int
pygpgme_context_init(PyGpgmeContext *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "", kwlist))
        return -1;

    if (self->ctx != NULL) {
        PyErr_SetString(PyExc_ValueError, "context already initialised");
        return -1;
    }

    if (pygpgme_check_error(gpgme_new(&self->ctx)))
        return -1;

    return 0;
}

static PyObject *
pygpgme_context_get_protocol(PyGpgmeContext *self)
{
    return PyInt_FromLong(gpgme_get_protocol(self->ctx));
}

static int
pygpgme_context_set_protocol(PyGpgmeContext *self, PyObject *value)
{
    gpgme_protocol_t protocol;

    protocol = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;

    if (pygpgme_check_error(gpgme_set_protocol(self->ctx, protocol)))
        return -1;

    return 0;
}

static PyObject *
pygpgme_context_get_armor(PyGpgmeContext *self)
{
    return PyBool_FromLong(gpgme_get_armor(self->ctx));
}

static int
pygpgme_context_set_armor(PyGpgmeContext *self, PyObject *value)
{
    int armor;

    armor = PyInt_AsLong(value) != 0;
    if (PyErr_Occurred())
        return -1;

    gpgme_set_armor(self->ctx, armor);
    return 0;
}

static PyObject *
pygpgme_context_get_textmode(PyGpgmeContext *self)
{
    return PyBool_FromLong(gpgme_get_textmode(self->ctx));
}

static int
pygpgme_context_set_textmode(PyGpgmeContext *self, PyObject *value)
{
    int textmode;

    textmode = PyInt_AsLong(value) != 0;
    if (PyErr_Occurred())
        return -1;

    gpgme_set_textmode(self->ctx, textmode);
    return 0;
}

static PyObject *
pygpgme_context_get_include_certs(PyGpgmeContext *self)
{
    return PyInt_FromLong(gpgme_get_include_certs(self->ctx));
}

static int
pygpgme_context_set_include_certs(PyGpgmeContext *self, PyObject *value)
{
    int nr_of_certs;

    nr_of_certs = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;

    gpgme_set_include_certs(self->ctx, nr_of_certs);
    return 0;
}

static PyObject *
pygpgme_context_get_keylist_mode(PyGpgmeContext *self)
{
    return PyInt_FromLong(gpgme_get_keylist_mode(self->ctx));
}

static int
pygpgme_context_set_keylist_mode(PyGpgmeContext *self, PyObject *value)
{
    gpgme_keylist_mode_t keylist_mode;

    keylist_mode = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;

    if (pygpgme_check_error(gpgme_set_keylist_mode(self->ctx, keylist_mode)))
        return -1;

    return 0;
}

static PyObject *
pygpgme_context_get_passphrase_cb(PyGpgmeContext *self)
{
    gpgme_passphrase_cb_t passphrase_cb;
    PyObject *callback;

    /* free the passphrase callback */
    gpgme_get_passphrase_cb(self->ctx, &passphrase_cb, (void **)&callback);
    if (passphrase_cb == pygpgme_passphrase_cb) {
        Py_INCREF(callback);
        return callback;
    } else {
        Py_RETURN_NONE;
    }
}

static int
pygpgme_context_set_passphrase_cb(PyGpgmeContext *self, PyObject *value)
{
    gpgme_passphrase_cb_t passphrase_cb;
    PyObject *callback;

    /* free the passphrase callback */
    gpgme_get_passphrase_cb(self->ctx, &passphrase_cb, (void **)&callback);
    if (passphrase_cb == pygpgme_passphrase_cb) {
        Py_DECREF(callback);
    }

    /* callback of None == unset */
    if (value == Py_None)
        value = NULL;

    if (value != NULL) {
        Py_INCREF(value);
        gpgme_set_passphrase_cb(self->ctx, pygpgme_passphrase_cb, value);
    } else {
        gpgme_set_passphrase_cb(self->ctx, NULL, NULL);
    }
    return 0;
}

static PyObject *
pygpgme_context_get_progress_cb(PyGpgmeContext *self)
{
    gpgme_progress_cb_t progress_cb;
    PyObject *callback;

    /* free the progress callback */
    gpgme_get_progress_cb(self->ctx, &progress_cb, (void **)&callback);
    if (progress_cb == pygpgme_progress_cb) {
        Py_INCREF(callback);
        return callback;
    } else {
        Py_RETURN_NONE;
    }
}

static int
pygpgme_context_set_progress_cb(PyGpgmeContext *self, PyObject *value)
{
    gpgme_progress_cb_t progress_cb;
    PyObject *callback;

    /* free the progress callback */
    gpgme_get_progress_cb(self->ctx, &progress_cb, (void **)&callback);
    if (progress_cb == pygpgme_progress_cb) {
        Py_DECREF(callback);
    }

    /* callback of None == unset */
    if (value == Py_None)
        value = NULL;

    if (value != NULL) {
        Py_INCREF(value);
        gpgme_set_progress_cb(self->ctx, pygpgme_progress_cb, value);
    } else {
        gpgme_set_progress_cb(self->ctx, NULL, NULL);
    }
    return 0;
}

static PyObject *
pygpgme_context_get_signers(PyGpgmeContext *self)
{
    PyObject *list, *tuple;
    gpgme_key_t key;
    int i;

    list = PyList_New(0);
    for (i = 0, key = gpgme_signers_enum(self->ctx, 0);
         key != NULL; key = gpgme_signers_enum(self->ctx, ++i)) {
        PyObject *item;

        item = pygpgme_key_new(key);
        gpgme_key_unref(key);
        if (item == NULL) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_Append(list, item);
        Py_DECREF(item);
    }
    tuple = PySequence_Tuple(list);
    Py_DECREF(list);
    return tuple;
}

static int
pygpgme_context_set_signers(PyGpgmeContext *self, PyObject *value)
{
    PyObject *signers = NULL;
    int i, length, ret = 0;

    signers = PySequence_Fast(value, "signers must be a sequence of keys");
    if (!signers) {
        ret = -1;
        goto end;
    }

    gpgme_signers_clear(self->ctx);
    length = PySequence_Fast_GET_SIZE(signers);
    for (i = 0; i < length; i++) {
        PyObject *item = PySequence_Fast_GET_ITEM(signers, i);

        if (!PyObject_TypeCheck(item, &PyGpgmeKey_Type)) {
            PyErr_SetString(PyExc_TypeError,
                            "signers must be a sequence of keys");
            ret = -1;
            goto end;
        }
        gpgme_signers_add(self->ctx, ((PyGpgmeKey *)item)->key);
    }

 end:
    Py_XDECREF(signers);
    return ret;
}

static PyGetSetDef pygpgme_context_getsets[] = {
    { "protocol", (getter)pygpgme_context_get_protocol,
      (setter)pygpgme_context_set_protocol },
    { "armor", (getter)pygpgme_context_get_armor,
      (setter)pygpgme_context_set_armor },
    { "textmode", (getter)pygpgme_context_get_textmode,
      (setter)pygpgme_context_set_textmode },
    { "include_certs", (getter)pygpgme_context_get_include_certs,
      (setter)pygpgme_context_set_include_certs },
    { "keylist_mode", (getter)pygpgme_context_get_keylist_mode,
      (setter)pygpgme_context_set_keylist_mode },
    { "passphrase_cb", (getter)pygpgme_context_get_passphrase_cb,
      (setter)pygpgme_context_set_passphrase_cb },
    { "progress_cb", (getter)pygpgme_context_get_progress_cb,
      (setter)pygpgme_context_set_progress_cb },
    { "signers", (getter)pygpgme_context_get_signers,
      (setter)pygpgme_context_set_signers },
    { NULL, (getter)0, (setter)0 }
};

/* XXX: set_locale */
static PyObject *
pygpgme_context_set_locale(PyGpgmeContext *self, PyObject *args)
{
    int category;
    const char *value;

    if (!PyArg_ParseTuple(args, "iz", &category, &value))
        return NULL;

    if (pygpgme_check_error(gpgme_set_locale(self->ctx, category, value)))
        return NULL;

    Py_RETURN_NONE;
}

/* the following don't seem to be used */
/* XXX: signers_clear */
/* XXX: signers_add */
/* XXX: signers_enum */

static PyObject *
pygpgme_context_get_key(PyGpgmeContext *self, PyObject *args)
{
    const char *fpr;
    int secret = 0;
    gpgme_error_t err;
    gpgme_key_t key;
    PyObject *ret;

    if (!PyArg_ParseTuple(args, "s|i", &fpr, &secret))
        return NULL;

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_get_key(self->ctx, fpr, &key, secret);
    Py_END_ALLOW_THREADS;

    if (pygpgme_check_error(err))
        return NULL;

    ret = pygpgme_key_new(key);
    gpgme_key_unref(key);
    return ret;
}

/* XXX: cancel -- not needed unless we wrap the async calls */

/* annotate exception with encrypt_result data */
static void
decode_encrypt_result(PyGpgmeContext *self)
{
    PyObject *err_type, *err_value, *err_traceback;
    gpgme_encrypt_result_t res;
    gpgme_invalid_key_t key;
    PyObject *list;

    PyErr_Fetch(&err_type, &err_value, &err_traceback);
    PyErr_NormalizeException(&err_type, &err_value, &err_traceback);

    if (!PyErr_GivenExceptionMatches(err_type, pygpgme_error))
        goto end;

    res = gpgme_op_encrypt_result(self->ctx);
    if (res == NULL)
        goto end;

    list = PyList_New(0);
    for (key = res->invalid_recipients; key != NULL; key = key->next) {
        PyObject *item, *err;

        err = pygpgme_error_object(key->reason);
        item = Py_BuildValue("(zN)", key->fpr, err);
        PyList_Append(list, item);
        Py_DECREF(item);
    }

    PyObject_SetAttrString(err_value, "invalid_recipients", list);
    Py_DECREF(list);

 end:
    PyErr_Restore(err_type, err_value, err_traceback);
}

static PyObject *
pygpgme_context_encrypt(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_recp, *py_plain, *py_cipher;
    int flags, i, length;
    gpgme_key_t *recp;
    gpgme_data_t plain, cipher;
    gpgme_error_t err;

    if (!PyArg_ParseTuple(args, "OiOO", &py_recp, &flags,
                          &py_plain, &py_cipher))
        return NULL;

    py_recp = PySequence_Fast(py_recp, "first argument must be a sequence");
    if (py_recp == NULL)
        return NULL;

    length = PySequence_Fast_GET_SIZE(py_recp);
    recp = malloc((length + 1) * sizeof (gpgme_key_t));
    for (i = 0; i < length; i++) {
        PyObject *item = PySequence_Fast_GET_ITEM(py_recp, i);

        if (!PyObject_TypeCheck(item, &PyGpgmeKey_Type)) {
            free(recp);
            Py_DECREF(py_recp);
            PyErr_SetString(PyExc_TypeError, "items in first argument must "
                            "be gpgme.Key objects");
            return NULL;
        }
        recp[i] = ((PyGpgmeKey *)item)->key;
    }
    recp[i] = NULL;

    if (pygpgme_data_new(&plain, py_plain)) {
        free(recp);
        Py_DECREF(py_recp);
        return NULL;    
    }
    if (pygpgme_data_new(&cipher, py_cipher)) {
        free(recp);
        Py_DECREF(py_recp);
        gpgme_data_release(plain);
        return NULL;    
    }

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_encrypt(self->ctx, recp, flags, plain, cipher);
    Py_END_ALLOW_THREADS;

    free(recp);
    Py_DECREF(py_recp);
    gpgme_data_release(plain);
    gpgme_data_release(cipher);

    if (pygpgme_check_error(err)) {
        decode_encrypt_result(self);
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
pygpgme_context_encrypt_sign(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_recp, *py_plain, *py_cipher;
    int flags, i, length;
    gpgme_key_t *recp;
    gpgme_data_t plain, cipher;
    gpgme_error_t err;
    gpgme_sign_result_t result;

    if (!PyArg_ParseTuple(args, "OiOO", &py_recp, &flags,
                          &py_plain, &py_cipher))
        return NULL;

    py_recp = PySequence_Fast(py_recp, "first argument must be a sequence");
    if (py_recp == NULL)
        return NULL;

    length = PySequence_Fast_GET_SIZE(py_recp);
    recp = malloc((length + 1) * sizeof (gpgme_key_t));
    for (i = 0; i < length; i++) {
        PyObject *item = PySequence_Fast_GET_ITEM(py_recp, i);

        if (!PyObject_TypeCheck(item, &PyGpgmeKey_Type)) {
            free(recp);
            Py_DECREF(py_recp);
            PyErr_SetString(PyExc_TypeError, "items in first argument must "
                            "be gpgme.Key objects");
            return NULL;
        }
        recp[i] = ((PyGpgmeKey *)item)->key;
    }
    recp[i] = NULL;

    if (pygpgme_data_new(&plain, py_plain)) {
        free(recp);
        Py_DECREF(py_recp);
        return NULL;    
    }
    if (pygpgme_data_new(&cipher, py_cipher)) {
        free(recp);
        Py_DECREF(py_recp);
        gpgme_data_release(plain);
        return NULL;    
    }

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_encrypt_sign(self->ctx, recp, flags, plain, cipher);
    Py_END_ALLOW_THREADS;

    free(recp);
    Py_DECREF(py_recp);
    gpgme_data_release(plain);
    gpgme_data_release(cipher);

    result = gpgme_op_sign_result(self->ctx);

    /* annotate exception */
    if (pygpgme_check_error(err)) {
        PyObject *err_type, *err_value, *err_traceback;
        PyObject *list;
        gpgme_invalid_key_t key;

        decode_encrypt_result(self);

        PyErr_Fetch(&err_type, &err_value, &err_traceback);
        PyErr_NormalizeException(&err_type, &err_value, &err_traceback);

        if (result == NULL)
            goto end;

        if (!PyErr_GivenExceptionMatches(err_type, pygpgme_error))
            goto end;

        list = PyList_New(0);
        for (key = result->invalid_signers; key != NULL; key = key->next) {
            PyObject *item, *err;

            err = pygpgme_error_object(key->reason);
            item = Py_BuildValue("(zN)", key->fpr, err);
            PyList_Append(list, item);
            Py_DECREF(item);
        }
        PyObject_SetAttrString(err_value, "invalid_signers", list);
        Py_DECREF(list);

        list = pygpgme_newsiglist_new(result->signatures);
        PyObject_SetAttrString(err_value, "signatures", list);
        Py_DECREF(list);
    end:
        PyErr_Restore(err_type, err_value, err_traceback);
        return NULL;
    }

    if (result)
        return pygpgme_newsiglist_new(result->signatures);
    else
        return PyList_New(0);
}

static void
decode_decrypt_result(PyGpgmeContext *self)
{
    PyObject *err_type, *err_value, *err_traceback;
    PyObject *value;
    gpgme_decrypt_result_t res;

    PyErr_Fetch(&err_type, &err_value, &err_traceback);
    PyErr_NormalizeException(&err_type, &err_value, &err_traceback);

    if (!PyErr_GivenExceptionMatches(err_type, pygpgme_error))
        goto end;

    res = gpgme_op_decrypt_result(self->ctx);
    if (res == NULL)
        goto end;

    if (res->unsupported_algorithm) {
        value = PyString_FromString(res->unsupported_algorithm);
    } else {
        Py_INCREF(Py_None);
        value = Py_None;
    }
    if (value) {
        PyObject_SetAttrString(err_value, "unsupported_algorithm", value);
        Py_DECREF(value);
    }

    value = PyBool_FromLong(res->wrong_key_usage);
    if (value) {
        PyObject_SetAttrString(err_value, "wrong_key_usage", value);
        Py_DECREF(value);
    }

 end:
    PyErr_Restore(err_type, err_value, err_traceback);
}

static PyObject *
pygpgme_context_decrypt(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_cipher, *py_plain;
    gpgme_data_t cipher, plain;
    gpgme_error_t err;

    if (!PyArg_ParseTuple(args, "OO", &py_cipher, &py_plain))
        return NULL;

    if (pygpgme_data_new(&cipher, py_cipher)) {
        return NULL;
    }

    if (pygpgme_data_new(&plain, py_plain)) {
        gpgme_data_release(cipher);
        return NULL;    
    }

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_decrypt(self->ctx, cipher, plain);
    Py_END_ALLOW_THREADS;

    gpgme_data_release(cipher);
    gpgme_data_release(plain);

    if (pygpgme_check_error(err)) {
        decode_decrypt_result(self);
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
pygpgme_context_decrypt_verify(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_cipher, *py_plain;
    gpgme_data_t cipher, plain;
    gpgme_error_t err;
    gpgme_verify_result_t result;

    if (!PyArg_ParseTuple(args, "OO", &py_cipher, &py_plain))
        return NULL;

    if (pygpgme_data_new(&cipher, py_cipher)) {
        return NULL;
    }

    if (pygpgme_data_new(&plain, py_plain)) {
        gpgme_data_release(cipher);
        return NULL;    
    }

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_decrypt_verify(self->ctx, cipher, plain);
    Py_END_ALLOW_THREADS;

    gpgme_data_release(cipher);
    gpgme_data_release(plain);

    if (pygpgme_check_error(err)) {
        decode_decrypt_result(self);
        return NULL;
    }

    result = gpgme_op_verify_result(self->ctx);

    /* annotate exception */
    if (pygpgme_check_error(err)) {
        PyObject *err_type, *err_value, *err_traceback;
        PyObject *list;

        PyErr_Fetch(&err_type, &err_value, &err_traceback);
        PyErr_NormalizeException(&err_type, &err_value, &err_traceback);

        if (result == NULL)
            goto end;

        if (!PyErr_GivenExceptionMatches(err_type, pygpgme_error))
            goto end;

        list = pygpgme_siglist_new(result->signatures);
        PyObject_SetAttrString(err_value, "signatures", list);
        Py_DECREF(list);
    end:
        PyErr_Restore(err_type, err_value, err_traceback);
        return NULL;
    }

    if (result)
        return pygpgme_siglist_new(result->signatures);
    else
        return PyList_New(0);
}

static PyObject *
pygpgme_context_sign(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_plain, *py_sig;
    gpgme_data_t plain, sig;
    int sig_mode = GPGME_SIG_MODE_NORMAL;
    gpgme_error_t err;
    gpgme_sign_result_t result;

    if (!PyArg_ParseTuple(args, "OO|i", &py_plain, &py_sig, &sig_mode))
        return NULL;

    if (pygpgme_data_new(&plain, py_plain))
        return NULL;    

    if (pygpgme_data_new(&sig, py_sig)) {
        gpgme_data_release(plain);
        return NULL;    
    }

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_sign(self->ctx, plain, sig, sig_mode);
    Py_END_ALLOW_THREADS;

    gpgme_data_release(plain);
    gpgme_data_release(sig);

    result = gpgme_op_sign_result(self->ctx);

    /* annotate exception */
    if (pygpgme_check_error(err)) {
        PyObject *err_type, *err_value, *err_traceback;
        PyObject *list;
        gpgme_invalid_key_t key;

        PyErr_Fetch(&err_type, &err_value, &err_traceback);
        PyErr_NormalizeException(&err_type, &err_value, &err_traceback);

        if (result == NULL)
            goto end;

        if (!PyErr_GivenExceptionMatches(err_type, pygpgme_error))
            goto end;

        list = PyList_New(0);
        for (key = result->invalid_signers; key != NULL; key = key->next) {
            PyObject *item, *err;

            err = pygpgme_error_object(key->reason);
            item = Py_BuildValue("(zN)", key->fpr, err);
            PyList_Append(list, item);
            Py_DECREF(item);
        }
        PyObject_SetAttrString(err_value, "invalid_signers", list);
        Py_DECREF(list);

        list = pygpgme_newsiglist_new(result->signatures);
        PyObject_SetAttrString(err_value, "signatures", list);
        Py_DECREF(list);
    end:
        PyErr_Restore(err_type, err_value, err_traceback);
        return NULL;
    }

    if (result)
        return pygpgme_newsiglist_new(result->signatures);
    else
        return PyList_New(0);
}

static PyObject *
pygpgme_context_verify(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_sig, *py_signed_text, *py_plaintext;
    gpgme_data_t sig, signed_text, plaintext;
    gpgme_error_t err;
    gpgme_verify_result_t result;

    if (!PyArg_ParseTuple(args, "OOO", &py_sig, &py_signed_text,
                          &py_plaintext))
        return NULL;

    if (pygpgme_data_new(&sig, py_sig)) {
        return NULL;
    }
    if (pygpgme_data_new(&signed_text, py_signed_text)) {
        gpgme_data_release(sig);
        return NULL;    
    }
    if (pygpgme_data_new(&plaintext, py_plaintext)) {
        gpgme_data_release(sig);
        gpgme_data_release(signed_text);
        return NULL;    
    }

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_verify(self->ctx, sig, signed_text, plaintext);
    Py_END_ALLOW_THREADS;

    gpgme_data_release(sig);
    gpgme_data_release(signed_text);
    gpgme_data_release(plaintext);

    result = gpgme_op_verify_result(self->ctx);

    /* annotate exception */
    if (pygpgme_check_error(err)) {
        PyObject *err_type, *err_value, *err_traceback;
        PyObject *list;

        PyErr_Fetch(&err_type, &err_value, &err_traceback);
        PyErr_NormalizeException(&err_type, &err_value, &err_traceback);

        if (result == NULL)
            goto end;

        if (!PyErr_GivenExceptionMatches(err_type, pygpgme_error))
            goto end;

        list = pygpgme_siglist_new(result->signatures);
        PyObject_SetAttrString(err_value, "signatures", list);
        Py_DECREF(list);
    end:
        PyErr_Restore(err_type, err_value, err_traceback);
        return NULL;
    }

    if (result)
        return pygpgme_siglist_new(result->signatures);
    else
        return PyList_New(0);
}

static PyObject *
pygpgme_context_import(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_keydata, *result;
    gpgme_data_t keydata;
    gpgme_error_t err;

    if (!PyArg_ParseTuple(args, "O", &py_keydata))
        return NULL;

    if (pygpgme_data_new(&keydata, py_keydata))
        return NULL;

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_import(self->ctx, keydata);
    Py_END_ALLOW_THREADS;

    gpgme_data_release(keydata);
    result = pygpgme_import_result(self->ctx);
    if (pygpgme_check_error(err)) {
        PyObject *err_type, *err_value, *err_traceback;

        PyErr_Fetch(&err_type, &err_value, &err_traceback);
        PyErr_NormalizeException(&err_type, &err_value, &err_traceback);

        if (!PyErr_GivenExceptionMatches(err_type, pygpgme_error))
            goto end;

        if (result != NULL) {
            PyObject_SetAttrString(err_value, "result", result);
            Py_DECREF(result);
        }
    end:
        PyErr_Restore(err_type, err_value, err_traceback);
        return NULL;
    }
    return result;
}

static PyObject *
pygpgme_context_export(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_pattern, *py_keydata;
    const char *pattern;
    const char **patterns;
    int i, length;
    gpgme_data_t keydata;
    gpgme_error_t err;

    if (!PyArg_ParseTuple(args, "OO", &py_pattern, &py_keydata))
        return NULL;

    if (py_pattern == Py_None) {
        Py_INCREF(py_pattern);
        pattern = NULL;
        patterns = NULL;
    } else if (PyString_Check(py_pattern)) {
        Py_INCREF(py_pattern);
        pattern = PyString_AsString(py_pattern);
        patterns = NULL;
    } else {
        py_pattern = PySequence_Fast(py_pattern,
            "first argument must be a string or sequence of strings");
        if (py_pattern == NULL)
            return NULL;
        length = PySequence_Fast_GET_SIZE(py_pattern);
        pattern = NULL;
        patterns = malloc((length + 1) * sizeof(const char *));
        for (i = 0; i < length; i++) {
            PyObject *item = PySequence_Fast_GET_ITEM(py_pattern, i);

            if (!PyString_Check(item)) {
                PyErr_SetString(PyExc_TypeError,
                    "first argument must be a string or sequence of strings");
                free(patterns);
                Py_DECREF(py_pattern);
                return NULL;
            }
            patterns[i] = PyString_AsString(item);
        }
        patterns[i] = NULL;
    }

    if (pygpgme_data_new(&keydata, py_keydata)) {
        Py_DECREF(py_pattern);
        if (patterns)
            free(patterns);
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS;
    if (patterns)
        err = gpgme_op_export_ext(self->ctx, patterns, 0, keydata);
    else
        err = gpgme_op_export(self->ctx, pattern, 0, keydata);
    Py_END_ALLOW_THREADS;

    Py_DECREF(py_pattern);
    if (patterns)
        free(patterns);
    gpgme_data_release(keydata);
    if (pygpgme_check_error(err))
        return NULL;
    Py_RETURN_NONE;
}

// pygpgme_context_genkey

static PyObject *
pygpgme_context_delete(PyGpgmeContext *self, PyObject *args)
{
    PyGpgmeKey *key;
    int allow_secret = 0;
    gpgme_error_t err;

    if (!PyArg_ParseTuple(args, "O!|i", &PyGpgmeKey_Type, &key, &allow_secret))
        return NULL;

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_delete(self->ctx, key->key, allow_secret);
    Py_END_ALLOW_THREADS;

    if (pygpgme_check_error(err))
        return NULL;
    Py_RETURN_NONE;
}

static gpgme_error_t
pygpgme_edit_cb(void *user_data, gpgme_status_code_t status,
                const char *args, int fd)
{
    PyObject *callback, *ret;
    PyGILState_STATE state;
    gpgme_error_t err;

    state = PyGILState_Ensure();
    callback = (PyObject *)user_data;
    ret = PyObject_CallFunction(callback, "lzi", (long)status, args, fd);
    err = pygpgme_check_pyerror();
    Py_XDECREF(ret);
    PyGILState_Release(state);
    return err;
}

static PyObject *
pygpgme_context_edit(PyGpgmeContext *self, PyObject *args)
{
    PyGpgmeKey *key;
    PyObject *callback, *py_out;
    gpgme_data_t out;
    gpgme_error_t err;

    if (!PyArg_ParseTuple(args, "O!OO", &PyGpgmeKey_Type, &key, &callback,
                          &py_out))
        return NULL;

    if (pygpgme_data_new(&out, py_out))
        return NULL;

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_edit(self->ctx, key->key,
                        pygpgme_edit_cb, (void *)callback, out);
    Py_END_ALLOW_THREADS;

    gpgme_data_release(out);

    if (pygpgme_check_error(err))
        return NULL;
    Py_RETURN_NONE;
}

static PyObject *
pygpgme_context_card_edit(PyGpgmeContext *self, PyObject *args)
{
    PyGpgmeKey *key;
    PyObject *callback, *py_out;
    gpgme_data_t out;
    gpgme_error_t err;

    if (!PyArg_ParseTuple(args, "O!OO", &PyGpgmeKey_Type, &key, &callback,
                          &py_out))
        return NULL;

    if (pygpgme_data_new(&out, py_out))
        return NULL;

    Py_BEGIN_ALLOW_THREADS;
    err = gpgme_op_card_edit(self->ctx, key->key,
                             pygpgme_edit_cb, (void *)callback, out);
    Py_END_ALLOW_THREADS;

    gpgme_data_release(out);

    if (pygpgme_check_error(err))
        return NULL;
    Py_RETURN_NONE;
}

static PyObject *
pygpgme_context_keylist(PyGpgmeContext *self, PyObject *args)
{
    PyObject *py_pattern = Py_None;
    const char *pattern;
    const char **patterns;
    int secret_only = 0, i, length;
    gpgme_error_t err;
    PyGpgmeKeyIter *ret;

    if (!PyArg_ParseTuple(args, "|Oi", &py_pattern, &secret_only))
        return NULL;

    if (py_pattern == Py_None) {
        Py_INCREF(py_pattern);
        pattern = NULL;
        patterns = NULL;
    } else if (PyString_Check(py_pattern)) {
        Py_INCREF(py_pattern);
        pattern = PyString_AsString(py_pattern);
        patterns = NULL;
    } else {
        py_pattern = PySequence_Fast(py_pattern,
            "first argument must be a string or sequence of strings");
        if (py_pattern == NULL)
            return NULL;
        length = PySequence_Fast_GET_SIZE(py_pattern);
        pattern = NULL;
        patterns = malloc((length + 1) * sizeof(const char *));
        for (i = 0; i < length; i++) {
            PyObject *item = PySequence_Fast_GET_ITEM(py_pattern, i);

            if (!PyString_Check(item)) {
                PyErr_SetString(PyExc_TypeError,
                    "first argument must be a string or sequence of strings");
                free(patterns);
                Py_DECREF(py_pattern);
                return NULL;
            }
            patterns[i] = PyString_AsString(item);
        }
        patterns[i] = NULL;
    }

    Py_BEGIN_ALLOW_THREADS;
    if (patterns)
        err = gpgme_op_keylist_ext_start(self->ctx, patterns, secret_only, 0);
    else
        err = gpgme_op_keylist_start(self->ctx, pattern, secret_only);
    Py_END_ALLOW_THREADS;

    Py_DECREF(py_pattern);
    if (patterns)
        free(patterns);

    if (pygpgme_check_error(err))
        return NULL;

    /* return a KeyIter object */
    ret = PyObject_New(PyGpgmeKeyIter, &PyGpgmeKeyIter_Type);
    if (!ret)
        return NULL;
    Py_INCREF(self);
    ret->ctx = self;
    return (PyObject *)ret;
}

// pygpgme_context_trustlist

static PyMethodDef pygpgme_context_methods[] = {
    { "set_locale", (PyCFunction)pygpgme_context_set_locale, METH_VARARGS },
    { "get_key", (PyCFunction)pygpgme_context_get_key, METH_VARARGS },
    { "encrypt", (PyCFunction)pygpgme_context_encrypt, METH_VARARGS },
    { "encrypt_sign", (PyCFunction)pygpgme_context_encrypt_sign, METH_VARARGS },
    { "decrypt", (PyCFunction)pygpgme_context_decrypt, METH_VARARGS },
    { "decrypt_verify", (PyCFunction)pygpgme_context_decrypt_verify, METH_VARARGS },
    { "sign", (PyCFunction)pygpgme_context_sign, METH_VARARGS },
    { "verify", (PyCFunction)pygpgme_context_verify, METH_VARARGS },
    { "import_", (PyCFunction)pygpgme_context_import, METH_VARARGS },
    { "export", (PyCFunction)pygpgme_context_export, METH_VARARGS },
    // genkey
    { "delete", (PyCFunction)pygpgme_context_delete, METH_VARARGS },
    { "edit", (PyCFunction)pygpgme_context_edit, METH_VARARGS },
    { "card_edit", (PyCFunction)pygpgme_context_card_edit, METH_VARARGS },
    { "keylist", (PyCFunction)pygpgme_context_keylist, METH_VARARGS },
    // trustlist
    { NULL, 0, 0 }
};

PyTypeObject PyGpgmeContext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,
    "gpgme.Context",
    sizeof(PyGpgmeContext),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor)pygpgme_context_dealloc,
    .tp_init = (initproc)pygpgme_context_init,
    .tp_getset = pygpgme_context_getsets,
    .tp_methods = pygpgme_context_methods,
};
