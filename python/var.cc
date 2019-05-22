#include "var.h"
#include "common.h"
#include "varinfo.h"

using namespace std;
using namespace wreport::python;
using namespace wreport;

extern "C" {

static _Varinfo dummy_var;

static wrpy_Var* wrpy_var_create(const wreport::Varinfo& v)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, &wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v);
    return result;
}

static wrpy_Var* wrpy_var_create_i(const wreport::Varinfo& v, int val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, &wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v, val);
    return result;
}

static wrpy_Var* wrpy_var_create_d(const wreport::Varinfo& v, double val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, &wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v, val);
    return result;
}

static wrpy_Var* wrpy_var_create_c(const wreport::Varinfo& v, const char* val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, &wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v, val);
    return result;
}

static wrpy_Var* wrpy_var_create_s(const wreport::Varinfo& v, const std::string& val)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, &wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v, val);
    return result;
}

static wrpy_Var* wrpy_var_create_copy(const wreport::Var& v)
{
    wrpy_Var* result = PyObject_New(wrpy_Var, &wrpy_Var_Type);
    if (!result) return nullptr;
    new (&result->var) Var(v);
    return result;
}


static PyObject* wrpy_Var_code(wrpy_Var* self, void* closure)
{
    return wrpy_varcode_format(self->var.code());
}

static PyObject* wrpy_Var_isset(wrpy_Var* self, void* closure) {
    if (self->var.isset())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

static PyObject* wrpy_Var_info(wrpy_Var* self, void* closure) {
    return (PyObject*)varinfo_create(self->var.info());
}

#define PYFIXME (char*)
static PyGetSetDef wrpy_Var_getsetters[] = {
    {PYFIXME "code", (getter)wrpy_Var_code, nullptr, PYFIXME "variable code", nullptr },
    {PYFIXME "isset", (getter)wrpy_Var_isset, nullptr, PYFIXME "true if the value is set", nullptr },
    {PYFIXME "info", (getter)wrpy_Var_info, nullptr, PYFIXME "Varinfo for this variable", nullptr },
    {nullptr}
};
#undef PYFIXME

static PyObject* wrpy_Var_enqi(wrpy_Var* self)
{
    try {
        return PyLong_FromLong(self->var.enqi());
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Var_enqd(wrpy_Var* self)
{
    try {
        return PyFloat_FromDouble(self->var.enqd());
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Var_enqc(wrpy_Var* self)
{
    try {
        return PyUnicode_FromString(self->var.enqc());
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Var_enq(wrpy_Var* self)
{
    return var_value_to_python(self->var);
}

static PyObject* wrpy_Var_get_attrs(wrpy_Var* self)
{
    pyo_unique_ptr res(PyList_New(0));
    if (!res)
        return nullptr;

    try {
        for (const Var* a = self->var.next_attr(); a != nullptr; a = a->next_attr())
        {
            // Create an empty variable, then set value from the attribute. This is
            // to avoid copying the rest of the attribute chain for every attribute
            // we are returning
            py_unique_ptr<wrpy_Var> var(var_create(a->info()));
            if (!var)
                return nullptr;
            var.get()->var.setval(*a);

            if (PyList_Append(res, (PyObject*)var.get()) == -1)
                return nullptr;
        }
        return res.release();
    } WREPORT_CATCH_RETURN_PYO
}

#define PYFIXME (char**)
static PyObject* wrpy_Var_enqa(wrpy_Var* self, PyObject* args, PyObject* kw)
{
    static const char* kwlist[] = { "code", nullptr };
    const char* code;
    if (!PyArg_ParseTupleAndKeywords(args, kw, "s", PYFIXME kwlist, &code))
        return nullptr;

    try {
        const Var* attr = self->var.enqa(varcode_parse(code));
        if (!attr)
            Py_RETURN_NONE;
        return (PyObject*)var_create(*attr);
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Var_seta(wrpy_Var* self, PyObject* args, PyObject* kw)
{
    static const char* kwlist[] = { "var", nullptr };
    wrpy_Var* var;
    if (!PyArg_ParseTupleAndKeywords(args, kw, "O!", PYFIXME kwlist, &wrpy_Var_Type, &var))
        return nullptr;

    try {
        self->var.seta(var->var);
        Py_RETURN_NONE;
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Var_unseta(wrpy_Var* self, PyObject* args, PyObject* kw)
{
    static const char* kwlist[] = { "code", nullptr };
    const char* code;
    if (!PyArg_ParseTupleAndKeywords(args, kw, "s", PYFIXME kwlist, &code))
        return nullptr;

    try {
        self->var.unseta(varcode_parse(code));
        Py_RETURN_NONE;
    } WREPORT_CATCH_RETURN_PYO
}

static PyObject* wrpy_Var_get(wrpy_Var* self, PyObject* args, PyObject* kw)
{
    static const char* kwlist[] = { "default", nullptr };
    PyObject* def = Py_None;
    if (!PyArg_ParseTupleAndKeywords(args, kw, "|O", PYFIXME kwlist, &def))
        return nullptr;
    if (self->var.isset())
        return var_value_to_python(self->var);
    else
    {
        Py_INCREF(def);
        return def;
    }
}

static PyObject* wrpy_Var_format(wrpy_Var* self, PyObject* args, PyObject* kw)
{
    static const char* kwlist[] = { "default", nullptr };
    const char* def = "";
    if (!PyArg_ParseTupleAndKeywords(args, kw, "|s", PYFIXME kwlist, &def))
        return nullptr;
    std::string f = self->var.format(def);
    return PyUnicode_FromString(f.c_str());
}
#undef PYFIXME

static PyMethodDef wrpy_Var_methods[] = {
    {"enqi", (PyCFunction)wrpy_Var_enqi, METH_NOARGS, R"(
        enqi() -> long

        get the value of the variable, as an int
    )" },
    {"enqd", (PyCFunction)wrpy_Var_enqd, METH_NOARGS, R"(
        enqd() -> float

        get the value of the variable, as a float
    )" },
    {"enqc", (PyCFunction)wrpy_Var_enqc, METH_NOARGS, R"(
        enqc() -> str

        get the value of the variable, as a str
    )" },
    {"enq", (PyCFunction)wrpy_Var_enq, METH_NOARGS, R"(
        enq() -> str|float|long

        get the value of the variable, as int, float or str according the variable definition
    )" },
    {"enqa", (PyCFunction)wrpy_Var_enqa, METH_VARARGS | METH_KEYWORDS, R"(
        enqa(code: str) -> Optional[wreport.Var]

        get the variable for the attribute with the given code, or None if not found
    )" },
    {"seta", (PyCFunction)wrpy_Var_seta, METH_VARARGS | METH_KEYWORDS, R"(
        seta(var: wreport.Var) -> None

        set an attribute in the variable
    )" },
    {"unseta", (PyCFunction)wrpy_Var_unseta, METH_VARARGS | METH_KEYWORDS, R"(
        unseta(code: str) -> None

        unset the given attribute from the variable
    )" },
    {"get", (PyCFunction)wrpy_Var_get, METH_VARARGS | METH_KEYWORDS, R"(
        get(default=None) -> str|float|long|default

        get the value of the variable, with a default if it is unset
    )" },
    {"get_attrs", (PyCFunction)wrpy_Var_get_attrs, METH_NOARGS, R"(
        get_attrs() -> List[wreport.Var]

        get the attributes of this variable
    )" },
    {"format", (PyCFunction)wrpy_Var_format, METH_VARARGS | METH_KEYWORDS, R"(
        format(default="") -> str

        return a string with the formatted value of the variable
    )" },
    {nullptr}
};

static int wrpy_Var_init(wrpy_Var* self, PyObject* args, PyObject* kw)
{
    PyObject* varinfo_or_var = nullptr;
    PyObject* val = nullptr;
    if (!PyArg_ParseTuple(args, "O|O", &varinfo_or_var, &val))
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

static void wrpy_Var_dealloc(wrpy_Var* self)
{
    // Explicitly call destructor
    self->var.~Var();
    Py_TYPE(self)->tp_free(self);
}

static PyObject* wrpy_Var_str(wrpy_Var* self)
{
    std::string f = self->var.format("None");
    return PyUnicode_FromString(f.c_str());
}

static PyObject* wrpy_Var_repr(wrpy_Var* self)
{
    string res = "Var('";
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
    return PyUnicode_FromString(res.c_str());
}

static PyObject* wrpy_Var_richcompare(wrpy_Var* a, wrpy_Var* b, int op)
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


PyTypeObject wrpy_Var_Type = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "wreport.Var",              // tp_name
    sizeof(wrpy_Var),           // tp_basicsize
    0,                         // tp_itemsize
    (destructor)wrpy_Var_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)wrpy_Var_repr,    // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash
    0,                         // tp_call
    (reprfunc)wrpy_Var_str,     // tp_str
    0,                         // tp_getattro
    0,                         // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT,        // tp_flags
    R"(
        Var holds a measured value, which can be integer, float or string, and
        a `wreport.Varinfo`_ with all available information (description, unit,
        precision, ...) related to it.

        Var objects can be created from a `wreport.Varinfo`_ object, and an
        optional value. Omitting the value creates an unset variable.

        Examples::

            v = wreport.Var(table["B12101"], 32.5)
            # v.info returns detailed informations about the variable in a Varinfo object.
            print("%s: %s %s %s" % (v.code, str(v), v.info.unit, v.info.desc))
    )",                        // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    (richcmpfunc)wrpy_Var_richcompare, // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    wrpy_Var_methods,           // tp_methods
    0,                         // tp_members
    wrpy_Var_getsetters,        // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)wrpy_Var_init,    // tp_init
    0,                         // tp_alloc
    0,                         // tp_new
};

}

namespace wreport {
namespace python {

wrpy_Var* var_create(const wreport::Varinfo& v) { return wrpy_var_create(v); }
wrpy_Var* var_create(const wreport::Varinfo& v, int val) { return wrpy_var_create_i(v, val); }
wrpy_Var* var_create(const wreport::Varinfo& v, double val) { return wrpy_var_create_d(v, val); }
wrpy_Var* var_create(const wreport::Varinfo& v, const char* val) { return wrpy_var_create_c(v, val); }
wrpy_Var* var_create(const wreport::Var& v) { return wrpy_var_create_copy(v); }

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
            string val;
            if (string_from_python(o, val))
                return -1;
            var.sets(val);
        } else {
            string repr;
            if (object_repr(o, repr))
                return -1;
            string type_repr;
            if (object_repr((PyObject*)o->ob_type, type_repr))
                return -1;
            string errmsg = "Value " + repr + " must be an instance of int, long, float, str, bytes, or unicode, instead of " + type_repr;
            PyErr_SetString(PyExc_TypeError, errmsg.c_str());
            return -1;
        }
        return 0;
    } WREPORT_CATCH_RETURN_INT
}

int register_var(PyObject* m, wrpy_c_api& c_api)
{
    dummy_var.set_bufr(0, "Invalid variable", "?", 0, 1, 0, 1);

    wrpy_Var_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&wrpy_Var_Type) < 0)
        return 0;

    // Initialize the C api struct
    c_api.var_create = wrpy_var_create;
    c_api.var_create_i = wrpy_var_create_i;
    c_api.var_create_d = wrpy_var_create_d;
    c_api.var_create_c = wrpy_var_create_c;
    c_api.var_create_s = wrpy_var_create_s;
    c_api.var_create_copy = wrpy_var_create_copy;
    c_api.var_value_to_python = var_value_to_python;
    c_api.var_value_from_python = var_value_from_python;
    c_api.varinfo_type = &wrpy_Varinfo_Type;
    c_api.vartable_type = &wrpy_Vartable_Type;
    c_api.var_type = &wrpy_Var_Type;

    Py_INCREF(&wrpy_Var_Type);
    return PyModule_AddObject(m, "Var", (PyObject*)&wrpy_Var_Type);
}

}
}
