#ifndef WREPORT_PYTHON_VAR_H
#define WREPORT_PYTHON_VAR_H

#include <Python.h>
#include <wreport/var.h>

extern "C" {

typedef struct {
    PyObject_HEAD
    wreport::Var var;
} wrpy_Var;

PyAPI_DATA(PyTypeObject) wrpy_Var_Type;

#define wrpy_Var_Check(ob) \
    (Py_TYPE(ob) == &wrpy_Var_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &wrpy_Var_Type))
}

namespace wreport {
namespace python {

PyObject* var_value_to_python(const wreport::Var& v);
int var_value_from_python(PyObject* o, wreport::Var& var);

wrpy_Var* var_create(const wreport::Varinfo& v);
wrpy_Var* var_create(const wreport::Varinfo& v, int val);
wrpy_Var* var_create(const wreport::Varinfo& v, double val);
wrpy_Var* var_create(const wreport::Varinfo& v, const char* val);
wrpy_Var* var_create(const wreport::Var& v);

void register_var(PyObject* m);

}
}

#endif
