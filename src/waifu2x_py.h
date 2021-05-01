// #define PY_SSIZE_T_CLEAN

#ifndef WAIFU2X_PY_H
#define WAIFU2X_PY_H
#if _WIN32
    #include <python.h>
#else
    #include <Python.h>
#endif
#include "waifu2x_main.h"

static PyObject*
waifu2x_py_init(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_init_set(PyObject* self, PyObject* args, PyObject* kwargs);

static PyObject*
waifu2x_py_clear (PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_delete(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_add(PyObject* self, PyObject* args, PyObject* kwargs);

static PyObject*
waifu2x_py_load(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_stop(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_version(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_test(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_get_info(PyObject* self, PyObject* args);

PyMODINIT_FUNC
PyInit_waifu2x(void);

static PyMethodDef SpamMethods[] = {
    {"init",  waifu2x_py_init, METH_VARARGS,
     "Execute a shell command."},
    {"initSet",  (PyCFunction)waifu2x_py_init_set, METH_VARARGS | METH_KEYWORDS,
     "Execute a shell command."},
    {"add",  (PyCFunction)waifu2x_py_add, METH_VARARGS | METH_KEYWORDS,
     "Execute a shell command."},
    {"getGpuInfo",  (PyCFunction)waifu2x_py_get_info, METH_VARARGS,
     "Execute a shell command."},
    {"delete",  (PyCFunction)waifu2x_py_delete, METH_VARARGS,
     "Execute a shell command."},
    {"clear",  (PyCFunction)waifu2x_py_clear, METH_VARARGS,
     "Execute a shell command."},
    {"load",  waifu2x_py_load, METH_VARARGS,
     "Execute a shell command."},
    {"stop",  waifu2x_py_stop, METH_VARARGS,
     "Execute a shell command."},
    {"getVersion",  waifu2x_py_version, METH_VARARGS,
     "Execute a shell command."},
    {"test",  waifu2x_py_test, METH_VARARGS,
     "Execute a shell command."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef spammodule = {
    PyModuleDef_HEAD_INIT,
    "waifu2x",   /* name of module */
    "doc", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    SpamMethods
};

static bool IsInit = false;
static bool IsInitSet = false;
static const char* Version = "version: v1.0.0\ngit:https://github.com/tonquer/waifu2x-ncnn-vulkan-python \n";
#endif 