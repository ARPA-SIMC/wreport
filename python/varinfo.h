#ifndef WREPORT_PYTHON_VARINFO_H
#define WREPORT_PYTHON_VARINFO_H

#include <Python.h>
#include <wreport/varinfo.h>

extern "C" {

typedef struct {
    PyObject_HEAD
    wreport::Varinfo info;
} dpy_Varinfo;

PyAPI_DATA(PyTypeObject) dpy_Varinfo_Type;

#define dpy_Varinfo_Check(ob) \
    (Py_TYPE(ob) == &dpy_Varinfo_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &dpy_Varinfo_Type))

}

namespace wreport {
namespace python {

dpy_Varinfo* varinfo_create(const wreport::Varinfo& v);

void register_varinfo(PyObject* m);

}
}
#endif
