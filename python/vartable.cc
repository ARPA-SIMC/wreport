#include "vartable.h"
#include "varinfo.h"
#include "common.h"
#include "utils/values.h"
#include <wreport/vartable.h>
#include <wreport/tableinfo.h>
#include "config.h"

using namespace std;
using namespace wreport;
using namespace wreport::python;
using namespace wreport;

extern "C" {

static int wrpy_Vartable_init(wrpy_Vartable* self, PyObject* args, PyObject* kw)
{
    // People should not invoke Varinfo() as a constructor, but if they do,
    // this is better than a segfault later on
    PyErr_SetString(PyExc_NotImplementedError, "Vartable objects cannot be constructed explicitly");
    return -1;
}

static PyObject* wrpy_Vartable_pathname(wrpy_Vartable* self, void* closure)
{
    return PyUnicode_FromString(self->table->pathname().c_str());
}

#define PYFIXME (char*)
static PyGetSetDef wrpy_Vartable_getsetters[] = {
    {PYFIXME "pathname", (getter)wrpy_Vartable_pathname, nullptr, PYFIXME "name of the table", nullptr},
    {nullptr}
};
#undef PYFIXME

static PyObject* wrpy_Vartable_str(wrpy_Vartable* self)
{
    return PyUnicode_FromString(self->table->pathname().c_str());
}

static PyObject* wrpy_Vartable_repr(wrpy_Vartable* self)
{
    return PyUnicode_FromFormat("Vartable('%s')", self->table->pathname().c_str());
}

static int wrpy_Vartable_len(wrpy_Vartable* self)
{
    // TODO return self->table->size();
    return 0;
}

static PyObject* wrpy_Vartable_item(wrpy_Vartable* self, Py_ssize_t i)
{
    Py_RETURN_NONE;
}

static PyObject* wrpy_Vartable_getitem(wrpy_Vartable* self, PyObject* key)
{
    try {
        std::string varname = from_python<std::string>(key);
        return (PyObject*)varinfo_create(self->table->query(varcode_parse(varname.c_str())));
    } WREPORT_CATCH_RETURN_PYO
}

static int wrpy_Vartable_contains(wrpy_Vartable* self, PyObject *value)
{
    try {
        std::string varname = from_python<std::string>(value);
        return self->table->contains(varcode_parse(varname.c_str())) ? 1 : 0;
    } WREPORT_CATCH_RETURN_INT
}

static PyObject* wrpy_Vartable_load_bufr(PyTypeObject *type, PyObject *args)
{
    const char* pathname;
    if (!PyArg_ParseTuple(args, "s", &pathname))
        return nullptr;

    try {
        return (PyObject*)vartable_create(Vartable::load_bufr(pathname));
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Vartable_load_crex(PyTypeObject *type, PyObject *args)
{
    const char* pathname;
    if (!PyArg_ParseTuple(args, "s", &pathname))
        return nullptr;

    try {
        return (PyObject*)vartable_create(Vartable::load_crex(pathname));
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Vartable_get_bufr(PyTypeObject *type, PyObject *args, PyObject* kw)
{
    static const char* kwlist[] = {
        "basename", "originating_centre", "originating_subcentre",
        "master_table_number", "master_table_version_number",
        "master_table_version_number_local", nullptr };
    const char* basename = nullptr;
    int originating_centre = 0;
    int originating_subcentre = 0;
    int master_table_number = 0;
    int master_table_version_number = -1;
    int master_table_version_number_local = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kw, "|siiiii",
                const_cast<char**>(kwlist), &basename,
                &originating_centre, &originating_subcentre,
                &master_table_number, &master_table_version_number,
                &master_table_version_number_local))
        return nullptr;

    if (basename)
        try {
            return (PyObject*)vartable_create(Vartable::get_bufr(basename));
        } WREPORT_CATCH_RETURN_PYO

    if (master_table_version_number == -1)
        PyErr_SetString(PyExc_ValueError, "Please pass either basename or master_table_version_number");

    BufrTableID id(
        originating_centre, originating_subcentre, master_table_number,
        master_table_version_number, master_table_version_number_local);

    try {
        return (PyObject*)vartable_create(Vartable::get_bufr(id));
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Vartable_get_crex(PyTypeObject *type, PyObject *args, PyObject* kw)
{
    static const char* kwlist[] = {
        "basename", "edition_number", "originating_centre", "originating_subcentre",
        "master_table_number", "master_table_version_number",
        "master_table_version_number_bufr",
        "master_table_version_number_local", nullptr };
    const char* basename = nullptr;
    int edition_number = 2;
    int originating_centre = 0;
    int originating_subcentre = 0;
    int master_table_number = 0;
    int master_table_version_number = -1;
    int master_table_version_number_bufr = -1;
    int master_table_version_number_local = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kw, "|siiiiiii",
                const_cast<char**>(kwlist), &basename,
                &edition_number, &originating_centre, &originating_subcentre,
                &master_table_number, &master_table_version_number,
                &master_table_version_number_bufr,
                &master_table_version_number_local))
        return nullptr;

    if (basename)
        try {
            return (PyObject*)vartable_create(Vartable::get_crex(basename));
        } WREPORT_CATCH_RETURN_PYO

    if (master_table_version_number == -1 && master_table_version_number_bufr == -1)
        PyErr_SetString(PyExc_ValueError, "Please pass at least one of basename, master_table_version_number, or master_table_version_number_bufr");

    CrexTableID id(
        edition_number, originating_centre, originating_subcentre, master_table_number,
        master_table_version_number == -1 ? 0xff : master_table_version_number,
        master_table_version_number_bufr == -1 ? 0xff : master_table_version_number_bufr,
        master_table_version_number_local);

    try {
        return (PyObject*)vartable_create(Vartable::get_crex(id));
    } WREPORT_CATCH_RETURN_PYO
}

static PyMethodDef wrpy_Vartable_methods[] = {
    {"load_bufr", (PyCFunction)wrpy_Vartable_load_bufr, METH_VARARGS | METH_CLASS,
        R"(
            Vartable.load_bufr(pathname) -> wreport.Vartable

            Load BUFR information from a Table B file and return it as a
            wreport.Vartable.
        )" },
    {"load_crex", (PyCFunction)wrpy_Vartable_load_crex, METH_VARARGS | METH_CLASS,
        R"(
            Vartable.load_crex(pathname) -> wreport.Vartable

            Load CREX information from a Table B file and return it as a
            wreport.Vartable.
        )" },
    {"get_bufr", (PyCFunction)wrpy_Vartable_get_bufr, METH_VARARGS | METH_KEYWORDS | METH_CLASS,
        R"(
            Vartable.get_bufr(basename=None, originating_centre=0, originating_subcentre=0,
                    master_table_number=0, master_table_version_number=None,
                    master_table_version_number_local=0) -> wreport.Vartable

            Look up a table B file using the information given, then load BUFR
            information from it.

            You need to provide either basename or master_table_version_number.
        )" },
    {"get_crex", (PyCFunction)wrpy_Vartable_get_crex, METH_VARARGS | METH_KEYWORDS | METH_CLASS,
        R"(
            Vartable.get_crex(basename=None, edition_number=2,
                    originating_centre=0, originating_subcentre=0,
                    master_table_number=0, master_table_version_number=None,
                    master_table_version_number_bufr=None,
                    master_table_version_number_local=0) -> wreport.Vartable

            Look up a table B file using the information given, then load CREX
            information from it.

            You need to provide either basename or master_table_version_number
            or master_table_version_number_bufr.
        )" },
    {nullptr}
};


static PySequenceMethods wrpy_Vartable_sequence = {
    (lenfunc)wrpy_Vartable_len,        // sq_length
    0,                                // sq_concat
    0,                                // sq_repeat
    (ssizeargfunc)wrpy_Vartable_item,  // sq_item
    0,                                // sq_slice
    0,                                // sq_ass_item
    0,                                // sq_ass_slice
    (objobjproc)wrpy_Vartable_contains, // sq_contains
};

static PyMappingMethods wrpy_Vartable_mapping = {
    (lenfunc)wrpy_Vartable_len,         // __len__
    (binaryfunc)wrpy_Vartable_getitem,  // __getitem__
    0,                // __setitem__
};

PyTypeObject wrpy_Vartable_Type = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "wreport.Vartable",         // tp_name
    sizeof(wrpy_Vartable),  // tp_basicsize
    0,                         // tp_itemsize
    0,                         // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)wrpy_Vartable_repr, // tp_repr
    0,                         // tp_as_number
    &wrpy_Vartable_sequence,    // tp_as_sequence
    &wrpy_Vartable_mapping,     // tp_as_mapping
    0,                         // tp_hash
    0,                         // tp_call
    (reprfunc)wrpy_Vartable_str, // tp_str
    0,                         // tp_getattro
    0,                         // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    R"(
        Collection of Varinfo objects indexed by WMO BUFR/CREX table B code.

        A Vartable is instantiated by the name (without extension) of the table
        file installed in wreport's data directory (normally,
        ``/usr/share/wreport/``)::

            table = wreport.Vartable("B0000000000000023000")
            print(table["B12101"].desc)

            for i in table:
                print(i.code, i.desc)
    )", // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    wrpy_Vartable_methods,      // tp_methods
    0,                         // tp_members
    wrpy_Vartable_getsetters,   // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)wrpy_Vartable_init, // tp_init
    0,                         // tp_alloc
    0,                         // tp_new
};

}

namespace wreport {
namespace python {

wrpy_Vartable* vartable_create(const wreport::Vartable* table)
{
    wrpy_Vartable* result = PyObject_New(wrpy_Vartable, &wrpy_Vartable_Type);
    if (!result) return nullptr;
    result->table = table;
    return result;
}

void register_vartable(PyObject* m, wrpy_c_api& c_api)
{
    wrpy_Vartable_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&wrpy_Vartable_Type) < 0)
        throw PythonException();

    // Initialize the C api struct
    c_api.vartable_create = vartable_create;

    Py_INCREF(&wrpy_Vartable_Type);
    if (PyModule_AddObject(m, "Vartable", (PyObject*)&wrpy_Vartable_Type) == -1)
        throw PythonException();
}

}
}
