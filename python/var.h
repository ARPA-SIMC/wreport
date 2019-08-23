#ifndef WREPORT_PYTHON_VAR_H
#define WREPORT_PYTHON_VAR_H

#include <wreport/var.h>
#include <wreport/python.h>
#include "utils/core.h"

extern "C" {

#ifndef WREPORT_3_21_COMPAT
/// wreport.Var python object
typedef struct {
    PyObject_HEAD
    wreport::Var var;
} wrpy_Var;

/// wreport.Var python type
extern PyTypeObject* wrpy_Var_Type;
#endif

/// Check if an object is of wreport.Var type or subtype
#define wrpy_Var_Check(ob) \
    (Py_TYPE(ob) == wrpy_Var_Type || \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_Var_Type))

}


namespace wreport {
namespace python {

PyObject* var_create(const wreport::Varinfo& v);
PyObject* var_create(const wreport::Varinfo& v, int val);
PyObject* var_create(const wreport::Varinfo& v, double val);
PyObject* var_create(const wreport::Varinfo& v, const char* val);
PyObject* var_create(const wreport::Var& v);

PyObject* var_value_to_python(const wreport::Var& v);
int var_value_from_python(PyObject* o, wreport::Var& var);

void register_var(PyObject* m, wrpy_c_api& c_c_api);

}
}

#endif
