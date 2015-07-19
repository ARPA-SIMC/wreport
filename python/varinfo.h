#ifndef WREPORT_PYTHON_VARINFO_H
#define WREPORT_PYTHON_VARINFO_H

#include <Python.h>
#include <wreport/varinfo.h>
#include <wreport/python.h>

namespace wreport {
namespace python {

wrpy_Varinfo* varinfo_create(wreport::Varinfo v);

int register_varinfo(PyObject* m, wrpy_c_api& c_c_api);

}
}
#endif
