#include "waifu2x_py.h"


PyMODINIT_FUNC
PyInit_waifu2x_vulkan(void)
{
    PyObject* m;

    m = PyModule_Create(&spammodule);
    if (m == NULL)
        return NULL;
    std::string models[3] = { "CUNET", "ANIME_STYLE_ART_RGB", "PHOTO" };
    int index = 0;
    for (int j = 0; j < 3; j++)
    {
        std::string name = models[j];
        for (int i = -1; i <= 3; i++)
        {
            char modelName[256];
            char modelNameTTa[256];
            if (i == -1)
            {
                sprintf(modelName, "MODEL_%s_NO_NOISE", name.c_str());
            }
            else
            {
                sprintf(modelName, "MODEL_%s_NOISE%d", name.c_str(), i);
            }
            sprintf(modelNameTTa, "%s_TTA", modelName);
            PyModule_AddIntConstant(m, modelName, index++);
            PyModule_AddIntConstant(m, modelNameTTa, index++);
        }
    }
    for (int i = -1; i <= 3; i++)
    {
        char modelName[256];
        char modelNameTTa[256];
        if (i == -1)
        {
            sprintf(modelName, "MODEL_CUNET_NO_SCALE_NO_NOISE");
        }
        else
        {
            sprintf(modelName, "MODEL_CUNET_NO_SCALE_NOISE%d", i);
        }
        sprintf(modelNameTTa, "%s_TTA", modelName);
        PyModule_AddIntConstant(m, modelName, index++);
        PyModule_AddIntConstant(m, modelNameTTa, index++);
    }
    return m;
}

static PyObject*
waifu2x_py_init(PyObject* self, PyObject* args)
{
    if (IsInit)
    {
        return PyLong_FromLong(0);
    }
    int sts = waifu2x_init();
    if (!sts) IsInit = true;
    return PyLong_FromLong(sts);
}

static PyObject*
waifu2x_py_init_set(PyObject* self, PyObject* args, PyObject* kwargs)
{
    if (!IsInit) return PyLong_FromLong(-1);
    if (IsInitSet) return PyLong_FromLong(0);
    int gpuId = 0;
    int threadNum = 0;
    char* kwarg_names[] = { "gpuId","threadNum", NULL };
    int sts;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii", kwarg_names, &gpuId, &threadNum))
        return PyLong_FromLong(-1);

    sts = waifu2x_init_set(gpuId, threadNum);
    if (!sts) IsInitSet = true;
    return PyLong_FromLong(sts);
}


static PyObject*
waifu2x_py_clear(PyObject* self, PyObject* args)
{
    if (!IsInitSet)
    {
        return PyLong_FromLong(0);
    }
    waifu2x_clear();
    return PyLong_FromLong(0);
}


static PyObject*
waifu2x_py_set_debug(PyObject* self, PyObject* args)
{
    unsigned int isDebug;
    if (!PyArg_ParseTuple(args, "i", &isDebug))
        Py_RETURN_NONE;
    waifu2x_set_debug(bool(isDebug));
    return PyLong_FromLong(0);
}

static PyObject*
waifu2x_py_set_path(PyObject* self, PyObject* args)
{
    const char* modelPath = 0;
    int sts;
    if (!PyArg_ParseTuple(args, "|s", &modelPath))
        Py_RETURN_NONE;

    if (modelPath)
    {
        sts = waifu2x_init_path(modelPath);
    }
    else
    {
        PyObject* pyModule = PyImport_ImportModule("waifu2x_vulkan");
        PyObject* v = PyObject_GetAttrString(pyModule, "__file__");

        PyObject* pathModule = PyImport_ImportModule("os.path");
        PyObject* func = PyObject_GetAttrString(pathModule, "dirname");

        PyObject* pyargs = PyTuple_New(1);
        PyTuple_SET_ITEM(pyargs, 0, v);
        PyObject* rel = PyObject_CallObject(func, pyargs);

        const char* path = NULL;
        PyArg_Parse(rel, "s", &path);
        sts = waifu2x_init_path(path);
    }
    return PyLong_FromLong(sts);
}

static PyObject*
waifu2x_py_remove_wait(PyObject* self, PyObject* args, PyObject* kwargs)
{
    if (!IsInitSet)
    {
        Py_RETURN_NONE;
    }
    PyObject* bufobj;
    char* kwarg_names[] = { "backIds", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kwarg_names, &bufobj))
        Py_RETURN_NONE;

    int list_len = PyObject_Size(bufobj);
    if (list_len <= 0)
    {
        Py_RETURN_NONE;
    }
    std::set<int> taskIds;
    PyObject* list_item = NULL;
    int taskId;
    for (int i = 0; i < list_len; i++)
    {
        list_item = PyList_GetItem(bufobj, i);
        PyArg_Parse(list_item, "i", &taskId);
        taskIds.insert(taskId);
    }
    waifu2x_remove_wait(taskIds);
    return PyLong_FromLong(0);
}

static PyObject*
waifu2x_py_remove(PyObject* self, PyObject* args, PyObject* kwargs)
{
    if (!IsInitSet)
    {
        Py_RETURN_NONE;
    }
    PyObject* bufobj;

    char* kwarg_names[] = { "backIds", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kwarg_names, &bufobj))
        Py_RETURN_NONE;

    int list_len = PyObject_Size(bufobj);
    if (list_len <= 0)
    {
        Py_RETURN_NONE;
    }
    std::set<int> taskIds;
    PyObject* list_item = NULL;
    int taskId;
    for (int i = 0; i < list_len; i++)
    {
        list_item = PyList_GetItem(bufobj, i);
        PyArg_Parse(list_item, "i", &taskId);
        taskIds.insert(taskId);
    }
    waifu2x_remove(taskIds);
    return PyLong_FromLong(0);
}


static PyObject*
waifu2x_py_add(PyObject* self, PyObject* args, PyObject* kwargs)
{
    if (!IsInitSet)
    {
        return PyLong_FromLong(0);
    }
    const char* b = NULL;
    unsigned int size;
    int sts = 1;
    int callBack;
    int modelIndex = 0;
    const char* format = NULL;
    int width = 0;
    int high = 0;
    float scale = 0;

    char* kwarg_names[] = { "data","modelIndex","backId", "format", "width", "high", "scale", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y#ii|siif", kwarg_names, &b, &size, &modelIndex, &callBack, &format, &width, &high, &scale))
        return PyLong_FromLong(-2);
    //fprintf(stdout, "point:%p, size:%d, index:%d, back:%d, scale:%f \n", b, size, modelIndex, callBack, scale);
    if (!b)
    {
        return PyLong_FromLong(-3);
    }
    //b = (unsigned char* )PyBytes_AsString((PyObject*)c);
    unsigned char* data = NULL;

    data = (unsigned char*)malloc(size);
    memcpy(data, b, size);
    sts = waifu2x_addData(data, size, callBack, modelIndex, format, width, high, scale);
    return PyLong_FromLong(sts);
}
static PyObject*
waifu2x_py_get_info(PyObject* self, PyObject* args)
{
    if (!IsInit)
    {
        Py_RETURN_NONE;
    }
    int gpu_count = ncnn::get_gpu_count();
    if (gpu_count <= 0)
    {
        Py_RETURN_NONE;
    }
    PyObject* pyList = PyList_New(gpu_count);
    for (int i = 0; i < gpu_count; i++)
    {
        const char* name = ncnn::get_gpu_info(i).device_name();
        PyObject* item = Py_BuildValue("s", name);
        PyList_SetItem(pyList, i, item);
    }
    return pyList;
}

static PyObject*
waifu2x_py_load(PyObject* self, PyObject* args, PyObject* kwargs)
{
    if (!IsInitSet) Py_RETURN_NONE;
    void* out = NULL;
    unsigned long outSize = 0;
    unsigned int block;
    double tick = 0;
    int callBack;
    char* kwarg_names[] = { "block", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwarg_names, &block))
        Py_RETURN_NONE;
    PyThreadState* save;
    save = PyEval_SaveThread();
    int sts = waifu2x_getData(out, outSize, tick, callBack, block);
    if (sts <= 0)
    {
        Py_RETURN_NONE;
    }
    PyEval_RestoreThread(save);
    PyObject* data = Py_BuildValue("y#iid", (char*)out, outSize, sts, callBack, tick);
    if (out) free(out);
    return data;
}

static PyObject*
waifu2x_py_stop(PyObject* self, PyObject* args)
{
    if (!IsInit)
    {
        return PyLong_FromLong(0);
    }

    int sts = waifu2x_stop();
    IsInit = false;
    IsInitSet = false;
    return PyLong_FromLong(sts);
}


static PyObject*
waifu2x_py_version(PyObject* self, PyObject* args)
{
    PyObject* data = Py_BuildValue("s", Version);
    return data;
}
