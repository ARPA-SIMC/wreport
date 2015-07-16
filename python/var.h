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

struct wrpy_c_api {
    wrpy_Var* (*var_create)(const wreport::Varinfo&);
    wrpy_Var* (*var_create_i)(const wreport::Varinfo&, int);
    wrpy_Var* (*var_create_d)(const wreport::Varinfo&, double);
    wrpy_Var* (*var_create_c)(const wreport::Varinfo&, const char*);
    wrpy_Var* (*var_create_copy)(const wreport::Var&);
};

}

namespace wreport {
namespace python {

wrpy_Var* var_create(const wreport::Varinfo& v);
wrpy_Var* var_create(const wreport::Varinfo& v, int val);
wrpy_Var* var_create(const wreport::Varinfo& v, double val);
wrpy_Var* var_create(const wreport::Varinfo& v, const char* val);
wrpy_Var* var_create(const wreport::Var& v);

PyObject* var_value_to_python(const wreport::Var& v);
int var_value_from_python(PyObject* o, wreport::Var& var);

int register_var(PyObject* m);

}
}

#endif
