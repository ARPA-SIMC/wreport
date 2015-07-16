#ifndef WREPORT_PYTHON_VARINFO_H
#define WREPORT_PYTHON_VARINFO_H

#include <Python.h>
#include <wreport/varinfo.h>

extern "C" {

typedef struct {
    PyObject_HEAD
    wreport::Varinfo info;
} wrpy_Varinfo;

PyAPI_DATA(PyTypeObject) wrpy_Varinfo_Type;

#define wrpy_Varinfo_Check(ob) \
    (Py_TYPE(ob) == &wrpy_Varinfo_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &wrpy_Varinfo_Type))

}

namespace wreport {
namespace python {

wrpy_Varinfo* varinfo_create(wreport::Varinfo v);

void register_varinfo(PyObject* m);

}
}
#endif
