#include <Python.h>
#include "config.h"
#include "common.h"
#include "vartable.h"
#include "varinfo.h"
#include "var.h"

#if PY_MAJOR_VERSION >= 3
    #define PyInt_FromLong PyLong_FromLong
    #define PyInt_AsLong PyLong_AsLong
    #define PyInt_Check PyLong_Check
    #define Py_TPFLAGS_HAVE_ITER 0
#endif

using namespace std;
using namespace wreport;
using namespace wreport::python;

extern "C" {

#if 0
static PyObject* wreport_varinfo(PyTypeObject *type, PyObject *args, PyObject *kw)
{
    const char* var_name;
    if (!PyArg_ParseTuple(args, "s", &var_name))
        return NULL;
    return (PyObject*)varinfo_create(wreport::varinfo(resolve_varcode(var_name)));
}

static PyObject* wreport_var_uncaught(PyTypeObject *type, PyObject *args)
{
    const char* var_name;
    PyObject* val = 0;
    if (!PyArg_ParseTuple(args, "s|O", &var_name, &val))
        return NULL;
    if (val)
    {
        if (PyFloat_Check(val))
        {
            double v = PyFloat_AsDouble(val);
            if (v == -1.0 && PyErr_Occurred())
                return NULL;
            return (PyObject*)var_create(wreport::varinfo(resolve_varcode(var_name)), v);
        } else if (PyInt_Check(val)) {
            long v = PyInt_AsLong(val);
            if (v == -1 && PyErr_Occurred())
                return NULL;
            return (PyObject*)var_create(wreport::varinfo(resolve_varcode(var_name)), (int)v);
        } else if (
                PyUnicode_Check(val)
#if PY_MAJOR_VERSION >= 3
                || PyBytes_Check(val)
#else
                || PyString_Check(val)
#endif
                ) {
            string v;
            if (string_from_python(val, v))
                return NULL;
            return (PyObject*)var_create(wreport::varinfo(resolve_varcode(var_name)), v.c_str());
        } else if (val == Py_None) {
            return (PyObject*)var_create(wreport::varinfo(resolve_varcode(var_name)));
        } else {
            PyErr_SetString(PyExc_TypeError, "Expected int, float, str, unicode, or None");
            return NULL;
        }
    } else
        return (PyObject*)var_create(wreport::varinfo(resolve_varcode(var_name)));
}

static PyObject* wreport_var(PyTypeObject *type, PyObject *args)
{
    try {
        return wreport_var_uncaught(type, args);
    } catch (wreport::error& e) {
        return raise_wreport_exception(e);
    } catch (std::exception& se) {
        return raise_std_exception(se);
    }
}
#endif

static PyMethodDef wreport_methods[] = {
//    {"varinfo", (PyCFunction)wreport_varinfo, METH_VARARGS, "Query the DB-All.e variable table returning a Varinfo" },
//    {"var", (PyCFunction)wreport_var, METH_VARARGS, "Query the DB-All.e variable table returning a Var, optionally initialized with a value" },
    { NULL }
};

#if PY_MAJOR_VERSION >= 3
static PyModuleDef wreport_module = {
    PyModuleDef_HEAD_INIT,
    "_wreport",       /* m_name */
    "wreport Python library",  /* m_doc */
    -1,             /* m_size */
    wreport_methods, /* m_methods */
    NULL,           /* m_reload */
    NULL,           /* m_traverse */
    NULL,           /* m_clear */
    NULL,           /* m_free */

};
#endif

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit__wreport(void)
#else
PyMODINIT_FUNC init_wreport(void)
#endif
{
    using namespace wreport::python;

static wrpy_c_api c_api;
    memset(&c_api, 0, sizeof(wrpy_c_api));

    PyObject* m;

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&wreport_module);
#else
    m = Py_InitModule3("_wreport", wreport_methods,
            "wreport Python library.");
#endif

#if PY_MAJOR_VERSION >= 3
    if (register_varinfo(m, c_api))
        return nullptr;
    if (register_vartable(m, c_api))
        return nullptr;
    if (register_var(m, c_api))
        return nullptr;
#else
    register_vartable(m, c_api);
    register_varinfo(m, c_api);
    register_var(m, c_api);
#endif

    // Create a Capsule containing the API struct's address
    pyo_unique_ptr c_api_object(PyCapsule_New((void *)&c_api, "_wreport._C_API", nullptr));
#if PY_MAJOR_VERSION >= 3
    if (!c_api_object)
        return nullptr;
#endif

    int res = PyModule_AddObject(m, "_C_API", c_api_object.release());
#if PY_MAJOR_VERSION >= 3
    if (res)
        return nullptr;
#endif

#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}

}
