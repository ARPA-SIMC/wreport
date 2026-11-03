/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

// SPDX-License-Identifier: BSD-3-Clause

#ifndef WREPORT_PYTHON_DICT_H
#define WREPORT_PYTHON_DICT_H

#include "core.h"
#include "values.h"

namespace wreport::python {

template <typename T>
inline void set_dict(PyObject* dict, const char* key, const T& val)
{
    auto pyval = to_python(val);
    if (PyDict_SetItemString(dict, key, pyval))
        throw PythonException();
}

inline void set_dict(PyObject* dict, const char* key, PyObject* val)
{
    if (PyDict_SetItemString(dict, key, val))
        throw PythonException();
}

inline void set_dict(PyObject* dict, const char* key, pyo_unique_ptr& val)
{
    if (PyDict_SetItemString(dict, key, val))
        throw PythonException();
}

} // namespace wreport::python

#endif
