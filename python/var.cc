#include "var.h"
#include "common.h"
#include "varinfo.h"
#include "utils/type.h"
#include "utils/methods.h"
#include "utils/values.h"

using namespace std;
using namespace wreport::python;
using namespace wreport;

extern "C" {

PyTypeObject* wrpy_Var_Type = nullptr;

// TODO: return PyObject* when we remove legacy support
static wrpy_Var* wrpy_var_create(const wreport::Varinfo& v)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v);
    return result;
}

static PyObject* wrpy_var_create_i(const wreport::Varinfo& v, int val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v, val);
    return (PyObject*)result;
}

static PyObject* wrpy_var_create_d(const wreport::Varinfo& v, double val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v, val);
    return (PyObject*)result;
}

static PyObject* wrpy_var_create_c(const wreport::Varinfo& v, const char* val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v, val);
    return (PyObject*)result;
}

static PyObject* wrpy_var_create_s(const wreport::Varinfo& v, const std::string& val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v, val);
    return (PyObject*)result;
}

static PyObject* wrpy_var_create_v(const wreport::Varinfo& v, const wreport::Var& val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v);
    result->var.setval(val);
    return (PyObject*)result;
}

static PyObject* wrpy_var_create_copy(const wreport::Var& v)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v);
    return (PyObject*)result;
}

static PyObject* wrpy_var_create_move(wreport::Var&& v)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(std::move(v));
    return (PyObject*)result;
}

static wreport::Var* wrpy_var(PyObject* o)
{
    if (!wrpy_Var_Check(o))
    {
        PyErr_Format(PyExc_TypeError, "expected object of type wreport.Var, got %R", o);
        return nullptr;
    }
    return &((wrpy_Var*)o)->var;
}

}

namespace {

static _Varinfo dummy_var;


struct code : public Getter<code, wrpy_Var>
{
    constexpr static const char* name = "code";
    constexpr static const char* doc = "variable code";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return wrpy_varcode_format(self->var.code());
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct isset : public Getter<isset, wrpy_Var>
{
    constexpr static const char* name = "isset";
    constexpr static const char* doc = "true if the value is set";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            if (self->var.isset())
                Py_RETURN_TRUE;
            else
                Py_RETURN_FALSE;
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct info : public Getter<info, wrpy_Var>
{
    constexpr static const char* name = "info";
    constexpr static const char* doc = "Varinfo for this variable";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try {
            return (PyObject*)varinfo_create(self->var.info());
        } WREPORT_CATCH_RETURN_PYO;
    }
};

struct enqi : public MethNoargs<enqi, wrpy_Var>
{
    constexpr static const char* name = "enqi";
    constexpr static const char* signature = "";
    constexpr static const char* returns = "int";
    constexpr static const char* summary = "get the value of the variable, as an int";

    static PyObject* run(Impl* self)
    {
        try {
            return to_python(self->var.enqi());
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct enqd : public MethNoargs<enqd, wrpy_Var>
{
    constexpr static const char* name = "enqd";
    constexpr static const char* signature = "";
    constexpr static const char* returns = "float";
    constexpr static const char* summary = "get the value of the variable, as a float";

    static PyObject* run(Impl* self)
    {
        try {
            return to_python(self->var.enqd());
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct enqc : public MethNoargs<enqc, wrpy_Var>
{
    constexpr static const char* name = "enqc";
    constexpr static const char* signature = "";
    constexpr static const char* returns = "str";
    constexpr static const char* summary = "get the value of the variable, as a str";

    static PyObject* run(Impl* self)
    {
        try {
            return to_python(self->var.enqc());
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct enq : public MethNoargs<enq, wrpy_Var>
{
    constexpr static const char* name = "enq";
    constexpr static const char* signature = "";
    constexpr static const char* returns = "Union[str, float, int]";
    constexpr static const char* summary = "get the value of the variable, as int, float or str according the variable definition";

    static PyObject* run(Impl* self)
    {
        try {
            return var_value_to_python(self->var);
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct enqa : public MethKwargs<enqa, wrpy_Var>
{
    constexpr static const char* name = "enqa";
    constexpr static const char* signature = "code: str";
    constexpr static const char* returns = "Optional[wreport.Var]";
    constexpr static const char* summary =
        "get the variable for the attribute with the given code, or None if not found";

    static PyObject* run(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = { "code", nullptr };
        const char* code;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "s", const_cast<char**>(kwlist), &code))
            return nullptr;

        try {
            const Var* attr = self->var.enqa(varcode_parse(code));
            if (!attr)
                Py_RETURN_NONE;
            return (PyObject*)var_create(*attr);
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct seta : public MethKwargs<seta, wrpy_Var>
{
    constexpr static const char* name = "seta";
    constexpr static const char* signature = "var: wreport.Var";
    constexpr static const char* summary = "set an attribute in the variable";

    static PyObject* run(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = { "var", nullptr };
        wrpy_Var* var;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "O!", const_cast<char**>(kwlist), wrpy_Var_Type, &var))
            return nullptr;

        try {
            self->var.seta(var->var);
            Py_RETURN_NONE;
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct unseta : public MethKwargs<unseta, wrpy_Var>
{
    constexpr static const char* name = "unseta";
    constexpr static const char* signature = "code: str";
    constexpr static const char* summary = "unset the given attribute from the variable";

    static PyObject* run(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = { "code", nullptr };
        const char* code;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "s", const_cast<char**>(kwlist), &code))
            return nullptr;

        try {
            self->var.unseta(varcode_parse(code));
            Py_RETURN_NONE;
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct get_attrs : public MethNoargs<get_attrs, wrpy_Var>
{
    constexpr static const char* name = "get_attrs";
    constexpr static const char* returns = "List[wreport.Var]";
    constexpr static const char* summary = "get the attributes of this variable";

    static PyObject* run(Impl* self)
    {
        try {
            pyo_unique_ptr res(throw_ifnull(PyList_New(0)));

            for (const Var* a = self->var.next_attr(); a != nullptr; a = a->next_attr())
            {
                // Create an empty variable, then set value from the attribute. This is
                // to avoid copying the rest of the attribute chain for every attribute
                // we are returning
                py_unique_ptr<wrpy_Var> var((wrpy_Var*)var_create(a->info()));
                if (!var)
                    return nullptr;
                var.get()->var.setval(*a);

                if (PyList_Append(res, (PyObject*)var.get()) == -1)
                    return nullptr;
            }
            return res.release();
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct get : public MethKwargs<get, wrpy_Var>
{
    constexpr static const char* name = "get";
    constexpr static const char* signature = "default: Any=None";
    constexpr static const char* returns = "Union[str, float, long, Any]";
    constexpr static const char* summary = "get the value of the variable, with a default if it is unset";

    static PyObject* run(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = { "default", nullptr };
        PyObject* def = Py_None;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "|O", const_cast<char**>(kwlist), &def))
            return nullptr;

        try {
            if (self->var.isset())
                return var_value_to_python(self->var);
            else
            {
                Py_INCREF(def);
                return def;
            }
        } WREPORT_CATCH_RETURN_PYO
    }
};

struct format : public MethKwargs<format, wrpy_Var>
{
    constexpr static const char* name = "format";
    constexpr static const char* signature = "default: str=""";
    constexpr static const char* returns = "str";
    constexpr static const char* summary = "return a string with the formatted value of the variable";

    static PyObject* run(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = { "default", nullptr };
        const char* def = "";
        if (!PyArg_ParseTupleAndKeywords(args, kw, "|s", const_cast<char**>(kwlist), &def))
            return nullptr;

        try {
            return to_python(self->var.format(def));
        } WREPORT_CATCH_RETURN_PYO
    }
};


struct VarDef : public Type<VarDef, wrpy_Var>
{
    constexpr static const char* name = "Var";
    constexpr static const char* qual_name = "wreport.Var";
    constexpr static const char* doc = R"(
Var holds a measured value, which can be integer, float or string, and
a :class:`Varinfo` with all available information (description, unit,
precision, ...) related to it.

Var objects can be created from a :class:`Varinfo` object, and an
optional value. Omitting the value creates an unset variable.

Examples::

    v = wreport.Var(table["B12101"], 32.5)
    # v.info returns detailed informations about the variable in a Varinfo object.
    print("%s: %s %s %s" % (v.code, str(v), v.info.unit, v.info.desc))

**Constructor**: Var(varinfo: Union[wreport.Varinfo, wreport.Var], value: Union[str, int, float] = None)

:arg varinfo: :class:`Varinfo` or :class:`Var` to use to create the variable
:arg value: value for the variable
)";
    GetSetters<code, isset, info> getsetters;
    Methods<enqi, enqd, enqc, enq, enqa, seta, unseta, get_attrs, get, format> methods;

    static int _init(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = { "varinfo", "value", nullptr };
        PyObject* varinfo_or_var = nullptr;
        PyObject* val = nullptr;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "O|O", const_cast<char**>(kwlist), &varinfo_or_var, &val))
            return -1;

        try {
            if (wrpy_Varinfo_Check(varinfo_or_var))
            {
                if (val == nullptr)
                {
                    new (&self->var) Var(((const wrpy_Varinfo*)varinfo_or_var)->info);
                    return 0;
                }
                else
                {
                    new (&self->var) Var(((const wrpy_Varinfo*)varinfo_or_var)->info);
                    return var_value_from_python(val, self->var);
                }
            }
            else if (wrpy_Var_Check(varinfo_or_var))
            {
                new (&self->var) Var(((const wrpy_Var*)varinfo_or_var)->var);
                return 0;
            }
            else
            {
                new (&self->var) Var(&dummy_var);
                PyErr_SetString(PyExc_ValueError, "First argument to wreport.Var should be wreport.Varinfo or wreport.Var");
                return -1;
            }
        } WREPORT_CATCH_RETURN_INT
    }

    static void _dealloc(Impl* self)
    {
        // Explicitly call destructor
        self->var.~Var();
        Py_TYPE(self)->tp_free(self);
    }

    static PyObject* _str(Impl* self)
    {
        try {
            return to_python(self->var.format("None"));
        } WREPORT_CATCH_RETURN_PYO;
    }

    static PyObject* _repr(Impl* self)
    {
        try {
            std::string res = "Var('";
            res += varcode_format(self->var.code());
            res += "', ";
            if (self->var.isset())
                switch (self->var.info()->type)
                {
                    case Vartype::String:
                    case Vartype::Binary:
                        res += "'" + self->var.format() + "'";
                        break;
                    case Vartype::Integer:
                    case Vartype::Decimal:
                        res += self->var.format();
                        break;
                }
            else
                res += "None";
            res += ")";
            return to_python(res);
        } WREPORT_CATCH_RETURN_PYO;
    }

    static PyObject* _richcompare(wrpy_Var* a, wrpy_Var* b, int op)
    {
        PyObject *result;
        bool cmp;

        // Make sure both arguments are Vars.
        if (!(wrpy_Var_Check(a) && wrpy_Var_Check(b))) {
            result = Py_NotImplemented;
            goto out;
        }

        switch (op) {
            case Py_EQ: cmp = a->var == b->var; break;
            case Py_NE: cmp = a->var != b->var; break;
            default:
                result = Py_NotImplemented;
                goto out;
        }
        result = cmp ? Py_True : Py_False;

    out:
        Py_INCREF(result);
        return result;
    }
};

VarDef* var_def = nullptr;

}

namespace wreport {
namespace python {

PyObject* var_create(const wreport::Varinfo& v) { return (PyObject*)wrpy_var_create(v); }
PyObject* var_create(const wreport::Varinfo& v, int val) { return wrpy_var_create_i(v, val); }
PyObject* var_create(const wreport::Varinfo& v, double val) { return wrpy_var_create_d(v, val); }
PyObject* var_create(const wreport::Varinfo& v, const char* val) { return wrpy_var_create_c(v, val); }
PyObject* var_create(const wreport::Var& v) { return wrpy_var_create_copy(v); }

PyObject* var_value_to_python(const wreport::Var& v)
{
    try {
        switch (v.info()->type)
        {
            case Vartype::String:
                return PyUnicode_FromString(v.enqc());
            case Vartype::Binary:
                return PyBytes_FromString(v.enqc());
            case Vartype::Integer:
                return PyLong_FromLong(v.enqi());
            case Vartype::Decimal:
                return PyFloat_FromDouble(v.enqd());
        }
        Py_RETURN_TRUE;
    } WREPORT_CATCH_RETURN_PYO
}

int var_value_from_python(PyObject* o, wreport::Var& var)
{
    try {
        if (PyLong_Check(o))
        {
            var.seti(PyLong_AsLong(o));
        } else if (PyFloat_Check(o)) {
            var.setd(PyFloat_AsDouble(o));
        } else if (PyBytes_Check(o)) {
            var.setc(PyBytes_AsString(o));
        } else if (PyUnicode_Check(o)) {
            var.sets(from_python<std::string>(o));
        } else {
            std::string repr = object_repr(o);
            std::string type_repr = object_repr((PyObject*)o->ob_type);
            string errmsg = "Value " + repr + " must be an instance of int, long, float, str, bytes, or unicode, instead of " + type_repr;
            PyErr_SetString(PyExc_TypeError, errmsg.c_str());
            return -1;
        }
        return 0;
    } WREPORT_CATCH_RETURN_INT
}

void register_var(PyObject* m, wrpy_c_api& c_api)
{
    dummy_var.set_bufr(0, "Invalid variable", "?", 0, 1, 0, 1);

    var_def = new VarDef;
    var_def->define(wrpy_Var_Type, m);

    // Initialize the C api struct
    c_api.var_create = wrpy_var_create;
    c_api.var_create_i = wrpy_var_create_i;
    c_api.var_create_d = wrpy_var_create_d;
    c_api.var_create_c = wrpy_var_create_c;
    c_api.var_create_s = wrpy_var_create_s;
    c_api.var_create_v = wrpy_var_create_v;
    c_api.var_create_copy = wrpy_var_create_copy;
    c_api.var_value_to_python = var_value_to_python;
    c_api.var_value_from_python = var_value_from_python;
    c_api.var_type = wrpy_Var_Type;
    c_api.var_create_move = wrpy_var_create_move;
    c_api.var = wrpy_var;
}

}
}
