#include "files.h"
#include "common.h"
#include "utils/methods.h"
#include "utils/type.h"
#include "utils/values.h"

using namespace std;
using namespace wreport::python;
using namespace wreport;

namespace {

std::string get_fd_name(PyObject* o,
                        const std::string& name_hint = std::string())
{
    // First try reading o.name
    pyo_unique_ptr o_name(PyObject_GetAttrString(o, "name"));
    if (o_name)
    {
        if (!PyUnicode_Check(o_name))
            o_name.reset(throw_ifnull(PyObject_Str(o_name)));
        return from_python<std::filesystem::path>(o_name);
    }
    PyErr_Clear();

    if (!name_hint.empty())
        return name_hint;

    // Then try repr
    pyo_unique_ptr o_str(PyObject_Str(o));
    if (o_str)
        return from_python<std::string>(o_str);

    throw PythonException();
}

off_t get_fd_pos(PyObject* o)
{
    // First try calling o.tell()
    pyo_unique_ptr o_pos(PyObject_CallMethod(o, "tell", nullptr));
    if (o_pos)
    {
        if (!PyLong_Check(o_pos))
        {
            PyErr_SetString(PyExc_TypeError,
                            "tell() returned a non-int result");
            throw PythonException();
        }
        return from_python<int>(o_pos);
    }

    // If tell raised UnsupportedOperation, return 0
    pyo_unique_ptr py_io(throw_ifnull(PyImport_ImportModule("io")));
    pyo_unique_ptr unsupported_operation(
        throw_ifnull(PyObject_GetAttrString(py_io, "UnsupportedOperation")));
    if (PyErr_ExceptionMatches(unsupported_operation))
    {
        return 0;
    }

    // Else reraise
    throw PythonException();
}

} // namespace

namespace wreport::python {

InputFilePython::InputFilePython(PyObject* obj)
    : obj(obj), offset(get_fd_pos(obj))
{
    Py_XINCREF(obj);
}

InputFilePython::~InputFilePython()
{
    if (obj)
        Py_XDECREF(obj);
}

const std::string& InputFilePython::name() const
{
    if (cached_name.empty())
        cached_name = get_fd_name(obj);
    return cached_name;
}

char InputFilePython::getc(const char* what)
{
    // TODO: annotate exception with what
    pyo_unique_ptr res(throw_ifnull(PyObject_CallMethod(obj, "read", "i", 1)));

    if (PyBytes_Check(res))
    {
        char* py_buf;
        Py_ssize_t py_len;
        if (PyBytes_AsStringAndSize(res, &py_buf, &py_len) != 0)
            throw PythonException();
        if (py_len != 1)
            return EOF;
        return py_buf[0];
    }
    else if (PyUnicode_Check(res))
    {
        Py_ssize_t py_len;
        const char* py_buf =
            throw_ifnull(PyUnicode_AsUTF8AndSize(res, &py_len));
        if (py_len != 1)
            return EOF;
        return py_buf[0];
    }
    else
    {
        PyErr_Format(PyExc_TypeError,
                     "read() returned %R instead of bytes or str", res.get());
        throw PythonException();
    }
}

off_t InputFilePython::seek_past_signature(const char* sig, unsigned sig_len)
{
    unsigned got = 0;
    int c;

    errno = 0;

    while (got < sig_len && (c = getc("message signature")) != EOF)
    {
        ++offset;
        if (c == sig[got])
            got++;
        else
            got = 0;
    }

    if (errno != 0)
    {
        if (name().empty())
            error_system::throwf("looking for start of %.4s data", sig);
        else
            error_system::throwf("looking for start of %.4s data in %s:", sig,
                                 name().c_str());
    }

    if (got != sig_len)
    {
        // End of file: return accordingly
        return -1;
    }
    return offset - sig_len;
}

void InputFilePython::read(char* buf, size_t size, const char* what)
{
    // TODO: annotate exception with what
    pyo_unique_ptr res(
        throw_ifnull(PyObject_CallMethod(obj, "read", "i", (int)size)));

    if (PyBytes_Check(res))
    {
        char* py_buf;
        Py_ssize_t py_len;
        if (PyBytes_AsStringAndSize(res, &py_buf, &py_len) != 0)
            throw PythonException();
        if ((unsigned)py_len != size)
        {
            if (name().empty())
                error_consistency::throwf(
                    "cannot read %s (%zd bytes) (partial read)", what, size);
            else
                error_consistency::throwf(
                    "cannot read %s (%zd bytes) from %s (partial read)", what,
                    size, name().c_str());
        }
        memcpy(buf, py_buf, size);
        offset += size;
    }
    else if (PyUnicode_Check(res))
    {
        Py_ssize_t py_len;
        const char* py_buf =
            throw_ifnull(PyUnicode_AsUTF8AndSize(res, &py_len));
        if (py_len != size)
        {
            if (name().empty())
                error_consistency::throwf(
                    "cannot read %s (%zd bytes) (partial read)", what, size);
            else
                error_consistency::throwf(
                    "cannot read %s (%zd bytes) from %s (partial read)", what,
                    size, name().c_str());
        }
        memcpy(buf, py_buf, size);
        offset += size;
    }
    else
    {
        PyErr_Format(PyExc_TypeError,
                     "read() returned %R instead of bytes or str", res.get());
        throw PythonException();
    }
}

void InputFilePython::read_until_crex_terminator(std::string& out)
{
    // Read until "\+\+(\r|\n)+7777"
    const char* target           = "++\r\n7777";
    static const int target_size = 8;
    int got                      = 0;
    int c;

    errno = 0;
    while (got < 8 && (c = getc("CREX terminator")) != EOF)
    {
        if (target[got] == '\r' && (c == '\n' || c == '\r'))
            got++;
        else if (target[got] == '\n' && (c == '\n' || c == '\r'))
            ;
        else if (target[got] == '\n' && c == '7')
            got += 2;
        else if (c == target[got])
            got++;
        else
            got = 0;

        out += (char)c;
    }

    if (got != target_size)
    {
        if (name().empty())
            throw error_consistency("CREX message is incomplete");
        else
            error_consistency::throwf("CREX message is incomplete in %s",
                                      name().c_str());
    }
}

} // namespace wreport::python
