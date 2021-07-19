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
waifu2x_py_remove_wait(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_remove(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_add(PyObject* self, PyObject* args, PyObject* kwargs);

static PyObject*
waifu2x_py_load(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_stop(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_version(PyObject* self, PyObject* args);

static PyObject*
waifu2x_py_get_info(PyObject* self, PyObject* args);

PyMODINIT_FUNC
PyInit_waifu2x(void);

static PyMethodDef SpamMethods[] = {
    {"init",  waifu2x_py_init, METH_VARARGS,
     "init ncnn"},
    {"initSet",  (PyCFunction)waifu2x_py_init_set, METH_VARARGS | METH_KEYWORDS,
     "init setting"},
    {"add",  (PyCFunction)waifu2x_py_add, METH_VARARGS | METH_KEYWORDS,
     "add task"},
    {"getGpuInfo",  (PyCFunction)waifu2x_py_get_info, METH_VARARGS,
     "get gpu list"},
    {"remove",  (PyCFunction)waifu2x_py_remove, METH_VARARGS,
     "delete all task"},
    {"clear",  (PyCFunction)waifu2x_py_clear, METH_VARARGS,
     "clear all queue"},
    {"removeWaitProc",  (PyCFunction)waifu2x_py_remove_wait, METH_VARARGS,
     "delete proc task"},
    {"load",  waifu2x_py_load, METH_VARARGS,
     "load a complete task"},
    {"stop",  waifu2x_py_stop, METH_VARARGS,
     "kill thread"},
    {"getVersion",  waifu2x_py_version, METH_VARARGS,
     "get version"},
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
static const char* Version = "version: v1.0.1\ngit:https://github.com/tonquer/waifu2x-ncnn-vulkan-python \n";
#endif 