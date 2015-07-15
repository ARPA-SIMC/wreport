#include "varinfo.h"
#include "wreport/var.h"
#include "common.h"

#if PY_MAJOR_VERSION >= 3
    #define PyInt_FromLong PyLong_FromLong
    #define PyInt_AsLong PyLong_AsLong
#endif

using namespace wreport;
using namespace wreport::python;
using namespace wreport;

extern "C" {

static PyMethodDef dpy_Varinfo_methods[] = {
    {NULL}
};

static PyObject* dpy_Varinfo_type(dpy_Varinfo *self, void* closure)
{
    return PyUnicode_FromString(vartype_format(self->info->type));
}
static PyObject* dpy_Varinfo_code(dpy_Varinfo *self, void* closure)
{
    return PyUnicode_FromString(varcode_format(self->info->code).c_str());
}
static PyObject* dpy_Varinfo_len(dpy_Varinfo* self, void* closure) { return PyInt_FromLong(self->info->len); }
static PyObject* dpy_Varinfo_unit(dpy_Varinfo* self, void* closure) { return PyUnicode_FromString(self->info->unit); }
static PyObject* dpy_Varinfo_desc(dpy_Varinfo* self, void* closure) { return PyUnicode_FromString(self->info->desc); }
static PyObject* dpy_Varinfo_scale(dpy_Varinfo* self, void* closure) { return PyInt_FromLong(self->info->scale); }
static PyObject* dpy_Varinfo_bit_ref(dpy_Varinfo* self, void* closure) { return PyInt_FromLong(self->info->bit_ref); }
static PyObject* dpy_Varinfo_bit_len(dpy_Varinfo* self, void* closure) { return PyInt_FromLong(self->info->bit_len); }

static PyGetSetDef dpy_Varinfo_getsetters[] = {
    {"type", (getter)dpy_Varinfo_type, NULL, "return a string describing the type of the variable (string, binary, integer, decimal)", NULL },
    {"code", (getter)dpy_Varinfo_code, NULL, "variable code", NULL },
    {"len", (getter)dpy_Varinfo_len, NULL, "number of significant digits", NULL},
    {"unit", (getter)dpy_Varinfo_unit, NULL, "measurement unit", NULL},
    {"desc", (getter)dpy_Varinfo_desc, NULL, "description", NULL},
    {"scale", (getter)dpy_Varinfo_scale, NULL, "scale of the value as a power of 10", NULL},
    {"bit_ref", (getter)dpy_Varinfo_bit_ref, NULL, "reference value added after scaling, for BUFR decoding", NULL},
    {"bit_len", (getter)dpy_Varinfo_bit_len, NULL, "number of bits used to encode the value in BUFR", NULL},
    {NULL}
};

static int dpy_Varinfo_init(dpy_Varinfo* self, PyObject* args, PyObject* kw)
{
    // People should not invoke Varinfo() as a constructor, but if they do,
    // this is better than a segfault later on
    PyErr_SetString(PyExc_NotImplementedError, "Varinfo objects cannot be constructed explicitly");
    return -1;
}

static PyObject* dpy_Varinfo_str(dpy_Varinfo* self)
{
    return format_varcode(self->info->code);
}

static PyObject* dpy_Varinfo_repr(dpy_Varinfo* self)
{
    return PyUnicode_FromString(varcode_format(self->info->code).c_str());
}


PyTypeObject dpy_Varinfo_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "wreport.Varinfo",         // tp_name
    sizeof(dpy_Varinfo),       // tp_basicsize
    0,                         // tp_itemsize
    0,                         // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)dpy_Varinfo_repr, // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash
    0,                         // tp_call
    (reprfunc)dpy_Varinfo_str, // tp_str
    0,                         // tp_getattro
    0,                         // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    R"(
    Varinfo object holds all possible information about a variable, such as its
    measurement unit, description and number of significant digits.

    Varinfo objects cannot be instantiated directly, but then can be created
    from `wreport.Vartable`_ objects, or using the `wreport.varinfo()`_ function.
    function.
    )",                        // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    dpy_Varinfo_methods,       // tp_methods
    0,                         // tp_members
    dpy_Varinfo_getsetters,    // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)dpy_Varinfo_init, // tp_init
    0,                         // tp_alloc
    0,                         // tp_new
};

}

namespace wreport {
namespace python {

dpy_Varinfo* varinfo_create(Varinfo v)
{
    dpy_Varinfo* result = PyObject_New(dpy_Varinfo, &dpy_Varinfo_Type);
    if (!result) return NULL;
    result->info = v;
    return result;
}

void register_varinfo(PyObject* m)
{
    dpy_Varinfo_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&dpy_Varinfo_Type) < 0)
        return;

    Py_INCREF(&dpy_Varinfo_Type);
    PyModule_AddObject(m, "Varinfo", (PyObject*)&dpy_Varinfo_Type);
}

}
}
