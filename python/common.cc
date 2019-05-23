#include "common.h"
#include <Python.h>

using namespace wreport;

namespace wreport {
namespace python {

PyObject* wrpy_varcode_format(wreport::Varcode code)
{
    char buf[7];
    snprintf(buf, 7, "%c%02d%03d",
            WR_VAR_F(code) == 0 ? 'B' :
            WR_VAR_F(code) == 1 ? 'R' :
            WR_VAR_F(code) == 2 ? 'C' :
            WR_VAR_F(code) == 3 ? 'D' : '?',
            WR_VAR_X(code), WR_VAR_Y(code));
    return PyUnicode_FromString(buf);
}

void set_wreport_exception(const wreport::error& e)
{
    switch (e.code())
    {
        case WR_ERR_NONE:
            PyErr_SetString(PyExc_SystemError, e.what());
            break;
        case WR_ERR_NOTFOUND:    // Item not found
            PyErr_SetString(PyExc_KeyError, e.what());
            break;
        case WR_ERR_TYPE:        // Wrong variable type
            PyErr_SetString(PyExc_TypeError, e.what());
            break;
        case WR_ERR_ALLOC:       // Cannot allocate memory
            PyErr_SetString(PyExc_MemoryError, e.what());
            break;
        case WR_ERR_ODBC:        // ODBC error
            PyErr_SetString(PyExc_OSError, e.what());
            break;
        case WR_ERR_HANDLES:     // Handle management error
            PyErr_SetString(PyExc_SystemError, e.what());
            break;
        case WR_ERR_TOOLONG:     // Buffer is too short to fit data
            PyErr_SetString(PyExc_ValueError, e.what());
            break;
        case WR_ERR_SYSTEM:      // Error reported by the system
            PyErr_SetString(PyExc_OSError, e.what());
            break;
        case WR_ERR_CONSISTENCY: // Consistency check failed
            PyErr_SetString(PyExc_RuntimeError, e.what());
            break;
        case WR_ERR_PARSE:       // Parse error
            PyErr_SetString(PyExc_ValueError, e.what());
            break;
        case WR_ERR_WRITE:       // Write error
            PyErr_SetString(PyExc_RuntimeError, e.what());
            break;
        case WR_ERR_REGEX:       // Regular expression error
            PyErr_SetString(PyExc_ValueError, e.what());
            break;
        case WR_ERR_UNIMPLEMENTED: // Feature not implemented
            PyErr_SetString(PyExc_NotImplementedError, e.what());
            break;
        case WR_ERR_DOMAIN:      // Value outside acceptable domain
            PyErr_SetString(PyExc_OverflowError, e.what());
            break;
        default:
            PyErr_Format(PyExc_SystemError, "unhandled exception with code %d: %s", e.code(), e.what());
            break;
    }
}

PyObject* raise_wreport_exception(const wreport::error& e)
{
    set_wreport_exception(e);
    return nullptr;
}

void set_std_exception(const std::exception& e)
{
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

PyObject* raise_std_exception(const std::exception& e)
{
    set_std_exception(e);
    return NULL;
}

int string_from_python(PyObject* o, std::string& out)
{
    if (PyBytes_Check(o)) {
        const char* v = PyBytes_AsString(o);
        if (v == NULL) return -1;
        out = v;
        return 0;
    }
    if (PyUnicode_Check(o)) {
        const char* v = PyUnicode_AsUTF8(o);
        if (v == NULL) return -1;
        out = v;
        return 0;
    }
    PyErr_SetString(PyExc_TypeError, "value must be an instance of str, bytes or unicode");
    return -1;
}

int file_get_fileno(PyObject* o)
{
    // fileno_value = obj.fileno()
    pyo_unique_ptr fileno_meth(PyObject_GetAttrString(o, "fileno"));
    if (!fileno_meth) return -1;
    pyo_unique_ptr fileno_args(Py_BuildValue("()"));
    if (!fileno_args) return -1;
    PyObject* fileno_value = PyObject_Call(fileno_meth, fileno_args, NULL);
    if (!fileno_value)
    {
        if (PyErr_ExceptionMatches(PyExc_AttributeError) || PyErr_ExceptionMatches(PyExc_IOError))
            PyErr_Clear();
        return -1;
    }

    // fileno = int(fileno_value)
    if (!PyObject_TypeCheck(fileno_value, &PyLong_Type)) {
        PyErr_SetString(PyExc_ValueError, "fileno() function must return an integer");
        return -1;
    }

    return PyLong_AsLong(fileno_value);
}

PyObject* file_get_data(PyObject* o, char*&buf, Py_ssize_t& len)
{
    // Use read() instead
    pyo_unique_ptr read_meth(PyObject_GetAttrString(o, "read"));
    pyo_unique_ptr read_args(Py_BuildValue("()"));
    pyo_unique_ptr data(PyObject_Call(read_meth, read_args, NULL));
    if (!data) return nullptr;

    if (!PyObject_TypeCheck(data, &PyBytes_Type)) {
        PyErr_SetString(PyExc_ValueError, "read() function must return a bytes object");
        return nullptr;
    }
    if (PyBytes_AsStringAndSize(data, &buf, &len))
        return nullptr;

    return data.release();
}


int object_repr(PyObject* o, std::string& out)
{
    pyo_unique_ptr repr(PyObject_Repr(o));
    if (!repr) return -1;

    if (string_from_python(repr, out))
        return -1;

    return 0;
}

}
}
