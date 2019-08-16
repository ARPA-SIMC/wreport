#ifndef WREPORT_PYTHON_VARTABLE_H
#define WREPORT_PYTHON_VARTABLE_H

#include <wreport/python.h>

namespace wreport {
namespace python {

wrpy_Vartable* vartable_create(const wreport::Vartable* table);

void register_vartable(PyObject* m, wrpy_c_api& c_c_api);

}
}
#endif
