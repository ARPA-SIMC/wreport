#ifndef WREPORT_PYTHON_H
#define WREPORT_PYTHON_H

#define WREPORT_3_21_COMPAT

#ifdef WREPORT_3_21_COMPAT
// TODO: remove when DB-All.e as deployed does not need this anymore
#include <wreport/var.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern "C" {
/// wreport.Var python object
typedef struct
{
    PyObject_HEAD wreport::Var var;
} wrpy_Var;

/// wreport.Var python type
extern PyTypeObject* wrpy_Var_Type;
}
#endif

#include <string>
#include <wreport/fwd.h>

#ifndef PyObject_HEAD
// Forward-declare PyObjetc and PyTypeObject
// see https://mail.python.org/pipermail/python-dev/2003-August/037601.html
extern "C" {
struct _object;
typedef _object PyObject;
struct _typeobject;
typedef _typeobject PyTypeObject;
}
#endif

extern "C" {

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
struct wrpy_c_api
{

    // API version 1.x

    /// Create a new unset wreport.Var object
    // TODO: return PyObject* when we drop legacy support
    wrpy_Var* (*var_create)(const wreport::Varinfo&);

    /// Create a new wreport.Var object with an integer value
    PyObject* (*var_create_i)(const wreport::Varinfo&, int);

    /// Create a new wreport.Var object with a double value
    PyObject* (*var_create_d)(const wreport::Varinfo&, double);

    /// Create a new wreport.Var object with a C string value
    PyObject* (*var_create_c)(const wreport::Varinfo&, const char*);

    /// Create a new wreport.Var object with a std::string value
    PyObject* (*var_create_s)(const wreport::Varinfo&, const std::string&);

    /// Create a new wreport.Var object as a copy of an existing var
    PyObject* (*var_create_copy)(const wreport::Var&);

    /// Read the value of a variable as a new Python object
    PyObject* (*var_value_to_python)(const wreport::Var&);

    /// Set the value of a variable from a Python object (borrowed reference)
    int (*var_value_from_python)(PyObject* o, wreport::Var&);

    /// Create a wreport.Varinfo object from a C++ Varinfo
    PyObject* (*varinfo_create)(wreport::Varinfo);

    /// Create a wreport:Vartable object from a C++ Vartable
    PyObject* (*vartable_create)(const wreport::Vartable*);

    /// C API major version (updated on incompatible changes)
    unsigned version_major;

    /// C API minor version (updated on backwards-compatible changes)
    unsigned version_minor;

    /// Varinfo type
    PyTypeObject* varinfo_type;

    /// Vartable type
    PyTypeObject* vartable_type;

    /// Var type
    PyTypeObject* var_type;

    // API version 1.1

    /// Create a new wreport.Var object, moving an existing var
    PyObject* (*var_create_move)(wreport::Var&&);

    /// Return the variable for a wreport.Var object
    wreport::Var* (*var)(PyObject* o);

    /// Create a new wreport.Var object with the value from another variable
    PyObject* (*var_create_v)(const wreport::Varinfo&, const wreport::Var&);
};
}

#endif
