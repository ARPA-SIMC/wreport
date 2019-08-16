#ifndef WREPORT_PYTHON_VARINFO_H
#define WREPORT_PYTHON_VARINFO_H

#include <wreport/varinfo.h>
#include <wreport/python.h>

extern "C" {

/// wreport.Varinfo python object
typedef struct {
    PyObject_HEAD
    wreport::Varinfo info;
} wrpy_Varinfo;

/// wreport.Varinfo python type
extern PyTypeObject* wrpy_Varinfo_Type;

/// Check if an object is of wreport.Varinfo type or subtype
#define wrpy_Varinfo_Check(ob) \
    (Py_TYPE(ob) == wrpy_Varinfo_Type || \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_Varinfo_Type))

}


namespace wreport {
namespace python {

PyObject* varinfo_create(wreport::Varinfo v);

void register_varinfo(PyObject* m, wrpy_c_api& c_c_api);

}
}
#endif
