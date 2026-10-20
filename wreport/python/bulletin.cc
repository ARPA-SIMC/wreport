#include "bulletin.h"
#include "common.h"
#include "files.h"
#include "utils/methods.h"
#include "utils/type.h"
#include "utils/values.h"

using namespace std;
using namespace wreport::python;
using namespace wreport;

extern "C" {

struct RawBufrIterator : public Type<RawBufrIterator, wrpy_RawBufrIterator>
{
    constexpr static const char* name      = "RawBufrIterator";
    constexpr static const char* qual_name = "wreport.RawBufrIterator";
    constexpr static const char* doc       = R"(
Read a file, generating all BUFR messages that it contains.

Each message is generated as a tuple ``(offset: int, message: str)``.

Messages are raw strings, which need to be decoded to access their contents.
)";

    GetSetters<> getsetters;
    Methods<> methods;

    static void _dealloc(Impl* self)
    {
        if (self->file)
        {
            delete self->file;
            self->file = nullptr;
        }
        Py_TYPE(self)->tp_free(self);
    }

    static int _init(wrpy_RawBufrIterator* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = {"file", NULL};
        PyObject* arg_file          = nullptr;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "O", pass_kwlist(kwlist),
                                         &arg_file))
            return -1;

        try
        {
            self->file = new InputFilePython(arg_file);
        }
        WREPORT_CATCH_RETURN_INT;
        return 0;
    }

    static PyObject* _iter(wrpy_RawBufrIterator* self)
    {
        Py_INCREF(self);
        return (PyObject*)self;
    }

    static PyObject* _iternext(wrpy_RawBufrIterator* self)
    {
        std::string buf;
        off_t offset;
        try
        {
            if (!BufrBulletin::read_file(*self->file, buf, &offset))
            {
                PyErr_SetNone(PyExc_StopIteration);
                return nullptr;
            }

            pyo_unique_ptr bytes(throw_ifnull(
                PyBytes_FromStringAndSize(buf.data(), buf.size())));

            pyo_unique_ptr res(
                throw_ifnull(PyTuple_Pack(2, to_python(offset), bytes.get())));
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

RawBufrIterator* raw_bufr_iterator = nullptr;
PyTypeObject* wrpy_RawBufrIterator_Type;

struct RawCrexIterator : public Type<RawCrexIterator, wrpy_RawCrexIterator>
{
    constexpr static const char* name      = "RawCrexIterator";
    constexpr static const char* qual_name = "wreport.RawCrexIterator";
    constexpr static const char* doc       = R"(
Read a file, generating all BUFR messages that it contains.

Each message is generated as a tuple ``(offset: int, message: str)``.

Messages are raw strings, which need to be decoded to access their contents.
)";

    GetSetters<> getsetters;
    Methods<> methods;

    static void _dealloc(Impl* self)
    {
        if (self->file)
        {
            delete self->file;
            self->file = nullptr;
        }
        Py_TYPE(self)->tp_free(self);
    }

    static int _init(wrpy_RawCrexIterator* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = {"file", NULL};
        PyObject* arg_file          = nullptr;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "O", pass_kwlist(kwlist),
                                         &arg_file))
            return -1;

        try
        {
            self->file = new InputFilePython(arg_file);
        }
        WREPORT_CATCH_RETURN_INT;
        return 0;
    }

    static PyObject* _iter(wrpy_RawCrexIterator* self)
    {
        Py_INCREF(self);
        return (PyObject*)self;
    }

    static PyObject* _iternext(wrpy_RawCrexIterator* self)
    {
        static bool done = false;
        std::string buf;
        off_t offset;
        try
        {
            if (!CrexBulletin::read_file(*self->file, buf, &offset))
            {
                PyErr_SetNone(PyExc_StopIteration);
                return nullptr;
            }

            pyo_unique_ptr py_offset(to_python(offset));
            pyo_unique_ptr py_bytes(throw_ifnull(
                PyBytes_FromStringAndSize(buf.data(), buf.size())));
            pyo_unique_ptr res(
                throw_ifnull(PyTuple_Pack(2, py_offset.get(), py_bytes.get())));

            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

RawCrexIterator* raw_crex_iterator = nullptr;
PyTypeObject* wrpy_RawCrexIterator_Type;

} // namespace

namespace wreport {
namespace python {

void register_bulletin(PyObject* m, wrpy_c_api& c_api)
{
    raw_bufr_iterator = new RawBufrIterator;
    raw_bufr_iterator->define(wrpy_RawBufrIterator_Type, m);

    raw_crex_iterator = new RawCrexIterator;
    raw_crex_iterator->define(wrpy_RawCrexIterator_Type, m);
}

} // namespace python
} // namespace wreport
