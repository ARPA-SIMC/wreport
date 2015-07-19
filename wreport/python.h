#ifndef WREPORT_PYTHON_H
#define WREPORT_PYTHON_H

#include <Python.h>
#include <wreport/varinfo.h>
#include <wreport/var.h>

namespace wreport {
struct Vartable;
}

extern "C" {

/// wreport.Varinfo python object
typedef struct {
    PyObject_HEAD
    wreport::Varinfo info;
} wrpy_Varinfo;

/// wreport.Varinfo python type
PyAPI_DATA(PyTypeObject) wrpy_Varinfo_Type;

/// Check if an object is of wreport.Varinfo type or subtype
#define wrpy_Varinfo_Check(ob) \
    (Py_TYPE(ob) == &wrpy_Varinfo_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &wrpy_Varinfo_Type))


/// wreport.Vartable python object
typedef struct {
    PyObject_HEAD
    const wreport::Vartable* table;
} wrpy_Vartable;

/// wreport.Vartable python type
PyAPI_DATA(PyTypeObject) wrpy_Vartable_Type;

/// Check if an object is of wreport.Vartable type or subtype
#define wrpy_Vartable_Check(ob) \
    (Py_TYPE(ob) == &wrpy_Vartable_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &wrpy_Vartable_Type))


/// wreport.Var python object
typedef struct {
    PyObject_HEAD
    wreport::Var var;
} wrpy_Var;

/// wreport.Var python type
PyAPI_DATA(PyTypeObject) wrpy_Var_Type;

/// Check if an object is of wreport.Var type or subtype
#define wrpy_Var_Check(ob) \
    (Py_TYPE(ob) == &wrpy_Var_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &wrpy_Var_Type))


/**
 * C++ functions exported by the wreport python bindings, to be used by other
 * C++ bindings.
 *
 * To use them, retrieve a pointer to the struct via the Capsule system:
 * \code
 * wrpy_c_api* wrpy = (wrpy_c_api*)PyCapsule_Import("_wreport._C_API", 0);
 * \endcode
 * 
 */
struct wrpy_c_api {
    /// Create a new unset wreport.Var object
    wrpy_Var* (*var_create)(const wreport::Varinfo&);

    /// Create a new wreport.Var object with an integer value
    wrpy_Var* (*var_create_i)(const wreport::Varinfo&, int);

    /// Create a new wreport.Var object with a double value
    wrpy_Var* (*var_create_d)(const wreport::Varinfo&, double);

    /// Create a new wreport.Var object with a C string value
    wrpy_Var* (*var_create_c)(const wreport::Varinfo&, const char*);

    /// Create a new wreport.Var object with a std::string value
    wrpy_Var* (*var_create_s)(const wreport::Varinfo&, const std::string&);

    /// Create a new wreport.Var object as a copy of an existing var
    wrpy_Var* (*var_create_copy)(const wreport::Var&);

    /// Read the value of a variable as a new Python object
    PyObject* (*var_value_to_python)(const wreport::Var&);

    /// Set the value of a variable from a Python object (borrowed reference)
    int (*var_value_from_python)(PyObject* o, wreport::Var&);

    /// Create a wreport.Varinfo object from a C++ Varinfo
    wrpy_Varinfo* (*varinfo_create)(wreport::Varinfo);

    /// Create a wreport:Vartable object from a C++ Vartable
    wrpy_Vartable* (*vartable_create)(const wreport::Vartable*);
};

}

#endif
