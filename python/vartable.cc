#include "vartable.h"
#include "varinfo.h"
#include "common.h"
#include "utils/type.h"
#include "utils/methods.h"
#include "utils/values.h"
#include <wreport/vartable.h>
#include <wreport/tableinfo.h>

using namespace std;
using namespace wreport;
using namespace wreport::python;
using namespace wreport;

extern "C" {

PyTypeObject* wrpy_Vartable_Type = nullptr;

}

namespace {

struct pathname : public Getter<pathname, wrpy_Vartable>
{
    constexpr static const char* name = "pathname";
    constexpr static const char* doc = "name of the table";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return to_python(self->table->pathname().c_str());
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct load_bufr : public ClassMethKwargs<load_bufr>
{
    constexpr static const char* name = "load_bufr";
    constexpr static const char* signature = "pathname: str";
    constexpr static const char* returns = "wreport.Vartable";
    constexpr static const char* summary = R"(
Load BUFR information from a Table B file and return it as a
wreport.Vartable.

:arg pathname: pathname of the file to load
)";

    static PyObject* run(PyTypeObject* cls, PyObject* args, PyObject* kw)
    {
        const char* pathname;
        if (!PyArg_ParseTuple(args, "s", &pathname))
            return nullptr;

        try {
            return (PyObject*)vartable_create(Vartable::load_bufr(pathname));
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct load_crex : public ClassMethKwargs<load_crex>
{
    constexpr static const char* name = "load_crex";
    constexpr static const char* signature = "pathname: str";
    constexpr static const char* returns = "wreport.Vartable";
    constexpr static const char* summary = R"(
Load CREX information from a Table B file and return it as a
wreport.Vartable.

:arg pathname: pathname of the file to load
)";

    static PyObject* run(PyTypeObject* cls, PyObject* args, PyObject* kw)
    {
        const char* pathname;
        if (!PyArg_ParseTuple(args, "s", &pathname))
            return nullptr;

        try {
            return (PyObject*)vartable_create(Vartable::load_crex(pathname));
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct get_bufr : public ClassMethKwargs<get_bufr>
{
    constexpr static const char* name = "get_bufr";
    constexpr static const char* signature =
        "basename: str=None, originating_centre: int=0, originating_subcentre: int=0,"
        "master_table_number: int=0, master_table_version_number: int=None, master_table_version_number_local: int=0";
    constexpr static const char* returns = "wreport.Vartable";
    constexpr static const char* summary = R"(
Look up a table B file using the information given, then load BUFR
information from it.
)";
    constexpr static const char* doc = R"(
You need to provide either basename or master_table_version_number.

:arg basename: load the table with the given name in ``/usr/share/wreport/``
:arg originating_centre: originating centre for the table data
:arg originating_subcentre: originating subcentre for the table data
:arg master_table_number: master table number for the table data
:arg master_table_version_number: master table version number for the table data
:arg master_table_version_number_local: local master table version number for the table data
)";

    static PyObject* run(PyTypeObject* cls, PyObject* args, PyObject* kw)
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

        try {
            if (basename)
                return (PyObject*)vartable_create(Vartable::get_bufr(basename));

            if (master_table_version_number == -1)
            {
                PyErr_SetString(PyExc_ValueError, "Please pass either basename or master_table_version_number");
                return nullptr;
            }

            BufrTableID id(
                originating_centre, originating_subcentre, master_table_number,
                master_table_version_number, master_table_version_number_local);

            return (PyObject*)vartable_create(Vartable::get_bufr(id));
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct get_crex : public ClassMethKwargs<get_crex>
{
    constexpr static const char* name = "get_crex";
    constexpr static const char* signature =
        "basename: str=None, edition_number=2, originating_centre: int=0, originating_subcentre: int=0,"
        "master_table_number: int=0, master_table_version_number: int=None,"
        "master_table_version_number_bufr: int=None, master_table_version_number_local: int=0";
    constexpr static const char* returns = "wreport.Vartable";
    constexpr static const char* summary = R"(
Look up a table B file using the information given, then load CREX
information from it.
)";
    constexpr static const char* doc = R"(
You need to provide either basename or master_table_version_number
or master_table_version_number_bufr.

:arg basename: load the table with the given name in ``/usr/share/wreport/``
:arg edition_number: edition number for the table data
:arg originating_centre: originating centre for the table data
:arg originating_subcentre: originating subcentre for the table data
:arg master_table_number: master table number for the table data
:arg master_table_version_number: master table version number for the table data
:arg master_table_version_number_bufr: BUFR master table version number for the table data
:arg master_table_version_number_local: local master table version number for the table data
)";

    static PyObject* run(PyTypeObject* cls, PyObject* args, PyObject* kw)
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

        try {
            if (basename)
                return (PyObject*)vartable_create(Vartable::get_crex(basename));

            if (master_table_version_number == -1 && master_table_version_number_bufr == -1)
            {
                PyErr_SetString(PyExc_ValueError, "Please pass at least one of basename, master_table_version_number, or master_table_version_number_bufr");
                return nullptr;
            }

            CrexTableID id(
                edition_number, originating_centre, originating_subcentre, master_table_number,
                master_table_version_number == -1 ? 0xff : master_table_version_number,
                master_table_version_number_bufr == -1 ? 0xff : master_table_version_number_bufr,
                master_table_version_number_local);

            return (PyObject*)vartable_create(Vartable::get_crex(id));
        } WREPORT_CATCH_RETURN_PYO
    }
};


struct VartableDef : public Type<VartableDef, wrpy_Vartable>

{
    constexpr static const char* name = "Vartable";
    constexpr static const char* qual_name = "wreport.Vartable";
    constexpr static const char* doc = R"(
Collection of Varinfo objects indexed by WMO BUFR/CREX table B code.

A Vartable is instantiated by one of the :meth:`get_bufr`, :meth:`get_crex`,
:meth:`load_bufr`, :meth:`load_crex` class methods::

    table = wreport.Vartable.get_bufr(master_table_version_number=23)
    print(table["B12101"].desc)
)";
    GetSetters<pathname> getsetters;
    Methods<get_bufr, get_crex, load_bufr, load_crex> methods;

    static void _dealloc(Impl* self)
    {
        Py_TYPE(self)->tp_free(self);
    }

    static PyObject* _str(Impl* self)
    {
        try {
            return to_python(self->table->pathname());
        } WREPORT_CATCH_RETURN_PYO;
    }

    static PyObject* _repr(Impl* self)
    {
        return PyUnicode_FromFormat("Vartable('%s')", self->table->pathname().c_str());
    }

    static int _init(Impl* self, PyObject* args, PyObject* kw)
    {
        // People should not invoke Varinfo() as a constructor, but if they do,
        // this is better than a segfault later on
        PyErr_SetString(PyExc_NotImplementedError, "Vartable objects cannot be constructed explicitly");
        return -1;
    }

    static int sq_length(Impl* self)
    {
        // TODO return self->table->size();
        return 0;
    }

    static PyObject* sq_item(Impl* self, Py_ssize_t i)
    {
        Py_RETURN_NONE;
    }

    static int sq_contains(Impl* self, PyObject *value)
    {
        try {
            std::string varname = from_python<std::string>(value);
            return self->table->contains(varcode_parse(varname.c_str())) ? 1 : 0;
        } WREPORT_CATCH_RETURN_INT
    }

    static int mp_length(Impl* self)
    {
        // TODO return self->table->size();
        return 0;
    }

    static PyObject* mp_subscript(wrpy_Vartable* self, PyObject* key)
    {
        try {
            std::string varname = from_python<std::string>(key);
            return (PyObject*)varinfo_create(self->table->query(varcode_parse(varname.c_str())));
        } WREPORT_CATCH_RETURN_PYO
    }
};

VartableDef* vartable_def = nullptr;

}

namespace wreport {
namespace python {

PyObject* vartable_create(const wreport::Vartable* table)
{
    wrpy_Vartable* result = PyObject_New(wrpy_Vartable, wrpy_Vartable_Type);
    if (!result) return nullptr;
    result->table = table;
    return (PyObject*)result;
}

void register_vartable(PyObject* m, wrpy_c_api& c_api)
{
    vartable_def = new VartableDef;
    vartable_def->define(wrpy_Vartable_Type, m);

    // Initialize the C api struct
    c_api.vartable_create = vartable_create;
    c_api.vartable_type = wrpy_Vartable_Type;
}

}
}
