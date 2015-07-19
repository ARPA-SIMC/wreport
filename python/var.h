#ifndef WREPORT_PYTHON_VAR_H
#define WREPORT_PYTHON_VAR_H

#include <Python.h>
#include <wreport/var.h>
#include <wreport/python.h>

namespace wreport {
namespace python {

wrpy_Var* var_create(const wreport::Varinfo& v);
wrpy_Var* var_create(const wreport::Varinfo& v, int val);
wrpy_Var* var_create(const wreport::Varinfo& v, double val);
wrpy_Var* var_create(const wreport::Varinfo& v, const char* val);
wrpy_Var* var_create(const wreport::Var& v);

PyObject* var_value_to_python(const wreport::Var& v);
int var_value_from_python(PyObject* o, wreport::Var& var);

int register_var(PyObject* m, wrpy_c_api& c_c_api);

}
}

#endif
