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

static PyMethodDef wrpy_Varinfo_methods[] = {
    {NULL}
};

static PyObject* wrpy_Varinfo_type(wrpy_Varinfo *self, void* closure)
{
    return PyUnicode_FromString(vartype_format(self->info->type));
}
static PyObject* wrpy_Varinfo_code(wrpy_Varinfo *self, void* closure)
{
    return wrpy_varcode_format(self->info->code);
}
static PyObject* wrpy_Varinfo_len(wrpy_Varinfo* self, void* closure) { return PyInt_FromLong(self->info->len); }
static PyObject* wrpy_Varinfo_unit(wrpy_Varinfo* self, void* closure) { return PyUnicode_FromString(self->info->unit); }
static PyObject* wrpy_Varinfo_desc(wrpy_Varinfo* self, void* closure) { return PyUnicode_FromString(self->info->desc); }
static PyObject* wrpy_Varinfo_scale(wrpy_Varinfo* self, void* closure) { return PyInt_FromLong(self->info->scale); }
static PyObject* wrpy_Varinfo_bit_ref(wrpy_Varinfo* self, void* closure) { return PyInt_FromLong(self->info->bit_ref); }
static PyObject* wrpy_Varinfo_bit_len(wrpy_Varinfo* self, void* closure) { return PyInt_FromLong(self->info->bit_len); }

static PyGetSetDef wrpy_Varinfo_getsetters[] = {
    {"type", (getter)wrpy_Varinfo_type, NULL, "return a string describing the type of the variable (string, binary, integer, decimal)", NULL },
    {"code", (getter)wrpy_Varinfo_code, NULL, "variable code", NULL },
    {"len", (getter)wrpy_Varinfo_len, NULL, "number of significant digits", NULL},
    {"unit", (getter)wrpy_Varinfo_unit, NULL, "measurement unit", NULL},
    {"desc", (getter)wrpy_Varinfo_desc, NULL, "description", NULL},
    {"scale", (getter)wrpy_Varinfo_scale, NULL, "scale of the value as a power of 10", NULL},
    {"bit_ref", (getter)wrpy_Varinfo_bit_ref, NULL, "reference value added after scaling, for BUFR decoding", NULL},
    {"bit_len", (getter)wrpy_Varinfo_bit_len, NULL, "number of bits used to encode the value in BUFR", NULL},
    {NULL}
};

static int wrpy_Varinfo_init(wrpy_Varinfo* self, PyObject* args, PyObject* kw)
{
    // People should not invoke Varinfo() as a constructor, but if they do,
    // this is better than a segfault later on
    PyErr_SetString(PyExc_NotImplementedError, "Varinfo objects cannot be constructed explicitly");
    return -1;
}

static PyObject* wrpy_Varinfo_str(wrpy_Varinfo* self)
{
    return wrpy_varcode_format(self->info->code);
}

static PyObject* wrpy_Varinfo_repr(wrpy_Varinfo* self)
{
    std::string res = "Varinfo('";
    res += varcode_format(self->info->code);
    res += "')";
    return PyUnicode_FromString(res.c_str());
}


PyTypeObject wrpy_Varinfo_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "wreport.Varinfo",         // tp_name
    sizeof(wrpy_Varinfo),       // tp_basicsize
    0,                         // tp_itemsize
    0,                         // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)wrpy_Varinfo_repr, // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash
    0,                         // tp_call
    (reprfunc)wrpy_Varinfo_str, // tp_str
    0,                         // tp_getattro
    0,                         // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    R"(
    Varinfo object holds all possible information about a variable, such as its
    measurement unit, description and number of significant digits.

    Varinfo objects cannot be instantiated directly, and are created by
    querying `wreport.Vartable`_ objects.
    )",                        // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    wrpy_Varinfo_methods,       // tp_methods
    0,                         // tp_members
    wrpy_Varinfo_getsetters,    // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)wrpy_Varinfo_init, // tp_init
    0,                         // tp_alloc
    0,                         // tp_new
};

}

namespace wreport {
namespace python {

wrpy_Varinfo* varinfo_create(Varinfo v)
{
    wrpy_Varinfo* result = PyObject_New(wrpy_Varinfo, &wrpy_Varinfo_Type);
    if (!result) return NULL;
    result->info = v;
    return result;
}

int register_varinfo(PyObject* m, wrpy_c_api& c_api)
{
    wrpy_Varinfo_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&wrpy_Varinfo_Type) < 0)
        return 0;

    // Initialize the C api struct
    c_api.varinfo_create = varinfo_create;

    Py_INCREF(&wrpy_Varinfo_Type);
    return PyModule_AddObject(m, "Varinfo", (PyObject*)&wrpy_Varinfo_Type);
}

}
}
