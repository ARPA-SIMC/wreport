#include "varinfo.h"
#include "wreport/var.h"
#include "common.h"
#include "utils/type.h"
#include "utils/methods.h"
#include "utils/values.h"

using namespace wreport;
using namespace wreport::python;
using namespace wreport;

extern "C" {

PyTypeObject* wrpy_Varinfo_Type = nullptr;

}

namespace {

struct type : public Getter<type, wrpy_Varinfo>
{
    constexpr static const char* name = "type";
    constexpr static const char* doc = "return a string describing the type of the variable (string, binary, integer, decimal)";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return to_python(vartype_format(self->info->type));
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct code : public Getter<code, wrpy_Varinfo>
{
    constexpr static const char* name = "code";
    constexpr static const char* doc = "variable code";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return wrpy_varcode_format(self->info->code);
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct len : public Getter<len, wrpy_Varinfo>
{
    constexpr static const char* name = "len";
    constexpr static const char* doc = "number of significant digits";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return to_python(self->info->len);
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct unit : public Getter<unit, wrpy_Varinfo>
{
    constexpr static const char* name = "unit";
    constexpr static const char* doc = "measurement unit";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return to_python(self->info->unit);
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct desc : public Getter<desc, wrpy_Varinfo>
{
    constexpr static const char* name = "desc";
    constexpr static const char* doc = "description";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return to_python(self->info->desc);
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct scale : public Getter<scale, wrpy_Varinfo>
{
    constexpr static const char* name = "scale";
    constexpr static const char* doc = "scale of the value as a power of 10";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return to_python(self->info->scale);
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct bit_ref : public Getter<bit_ref, wrpy_Varinfo>
{
    constexpr static const char* name = "bit_ref";
    constexpr static const char* doc = "reference value added after scaling, for BUFR decoding";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return to_python(self->info->bit_ref);
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct bit_len : public Getter<bit_len, wrpy_Varinfo>
{
    constexpr static const char* name = "bit_len";
    constexpr static const char* doc = "number of bits used to encode the value in BUFR";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return to_python(self->info->bit_len);
        } WREPORT_CATCH_RETURN_PYO;
    }
};


struct VarinfoDef : public Type<VarinfoDef, wrpy_Varinfo>
{
    constexpr static const char* name = "Varinfo";
    constexpr static const char* qual_name = "wreport.Varinfo";
    constexpr static const char* doc = R"(
Varinfo object holds all possible information about a variable, such as its
measurement unit, description and number of significant digits.

Varinfo objects cannot be instantiated directly, and are created by
querying :class:`Vartable` objects.
)";
    GetSetters<type, code, len, unit, desc, scale, bit_ref, bit_len> getsetters;
    Methods<> methods;

    static void _dealloc(Impl* self)
    {
        Py_TYPE(self)->tp_free(self);
    }

    static PyObject* _str(Impl* self)
    {
        try {
            return wrpy_varcode_format(self->info->code);
        } WREPORT_CATCH_RETURN_PYO;
    }

    static PyObject* _repr(Impl* self)
    {
        std::string res = "Varinfo('";
        res += varcode_format(self->info->code);
        res += "')";
        return PyUnicode_FromString(res.c_str());
    }

    static int _init(Impl* self, PyObject* args, PyObject* kw)
    {
        // People should not invoke Varinfo() as a constructor, but if they do,
        // this is better than a segfault later on
        PyErr_SetString(PyExc_NotImplementedError, "Varinfo objects cannot be constructed explicitly");
        return -1;
    }
};

VarinfoDef* varinfo_def = nullptr;

}

namespace wreport {
namespace python {

PyObject* varinfo_create(Varinfo v)
{
    wrpy_Varinfo* result = PyObject_New(wrpy_Varinfo, wrpy_Varinfo_Type);
    if (!result) return nullptr;
    result->info = v;
    return (PyObject*)result;
}

void register_varinfo(PyObject* m, wrpy_c_api& c_api)
{
    varinfo_def = new VarinfoDef;
    varinfo_def->define(wrpy_Varinfo_Type, m);

    // Initialize the C api struct
    c_api.varinfo_create = varinfo_create;
    c_api.varinfo_type = wrpy_Varinfo_Type;
}

}
}
