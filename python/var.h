#ifndef WREPORT_PYTHON_VAR_H
#define WREPORT_PYTHON_VAR_H

#include <Python.h>
#include <wreport/var.h>

extern "C" {

typedef struct {
    PyObject_HEAD
    wreport::Var var;
} dpy_Var;

PyAPI_DATA(PyTypeObject) dpy_Var_Type;

#define dpy_Var_Check(ob) \
    (Py_TYPE(ob) == &dpy_Var_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &dpy_Var_Type))
}

namespace wreport {
namespace python {

PyObject* var_value_to_python(const wreport::Var& v);
int var_value_from_python(PyObject* o, wreport::Var& var);

dpy_Var* var_create(const wreport::Varinfo& v);
dpy_Var* var_create(const wreport::Varinfo& v, int val);
dpy_Var* var_create(const wreport::Varinfo& v, double val);
dpy_Var* var_create(const wreport::Varinfo& v, const char* val);
dpy_Var* var_create(const wreport::Var& v);

void register_var(PyObject* m);

}
}

#endif
