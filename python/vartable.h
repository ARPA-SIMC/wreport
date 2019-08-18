#ifndef WREPORT_PYTHON_VARTABLE_H
#define WREPORT_PYTHON_VARTABLE_H

#include <wreport/python.h>
#include "utils/core.h"

extern "C" {

/// wreport.Vartable python object
typedef struct {
    PyObject_HEAD
    const wreport::Vartable* table;
} wrpy_Vartable;

/// wreport.Vartable python type
extern PyTypeObject* wrpy_Vartable_Type;

/// Check if an object is of wreport.Vartable type or subtype
#define wrpy_Vartable_Check(ob) \
    (Py_TYPE(ob) == wrpy_Vartable_Type || \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_Vartable_Type))

}


namespace wreport {
namespace python {

PyObject* vartable_create(const wreport::Vartable* table);

void register_vartable(PyObject* m, wrpy_c_api& c_c_api);

}
}
#endif
