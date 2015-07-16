#ifndef WREPORT_PYTHON_H
#define WREPORT_PYTHON_H

#include <Python.h>
#include <wreport/varinfo.h>
#include <wreport/var.h>

namespace wreport {
struct Vartable;
}

extern "C" {

/*
 * wreport::Varinfo bindings
 */
typedef struct {
    PyObject_HEAD
    wreport::Varinfo info;
} wrpy_Varinfo;

PyAPI_DATA(PyTypeObject) wrpy_Varinfo_Type;

#define wrpy_Varinfo_Check(ob) \
    (Py_TYPE(ob) == &wrpy_Varinfo_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &wrpy_Varinfo_Type))


/*
 * wreport::Vartable bindings
 */

typedef struct {
    PyObject_HEAD
    const wreport::Vartable* table;
} wrpy_Vartable;

PyAPI_DATA(PyTypeObject) wrpy_Vartable_Type;

#define wrpy_Vartable_Check(ob) \
    (Py_TYPE(ob) == &wrpy_Vartable_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &wrpy_Vartable_Type))


/*
 * wreport::Var bindings
 */

typedef struct {
    PyObject_HEAD
    wreport::Var var;
} wrpy_Var;

PyAPI_DATA(PyTypeObject) wrpy_Var_Type;

#define wrpy_Var_Check(ob) \
    (Py_TYPE(ob) == &wrpy_Var_Type || \
     PyType_IsSubtype(Py_TYPE(ob), &wrpy_Var_Type))


/*
 * Common parts
 */

struct wrpy_c_api {
    wrpy_Var* (*var_create)(const wreport::Varinfo&);
    wrpy_Var* (*var_create_i)(const wreport::Varinfo&, int);
    wrpy_Var* (*var_create_d)(const wreport::Varinfo&, double);
    wrpy_Var* (*var_create_c)(const wreport::Varinfo&, const char*);
    wrpy_Var* (*var_create_copy)(const wreport::Var&);
    PyObject* (*var_value_to_python)(const wreport::Var&);
    int (*var_value_from_python)(PyObject* o, wreport::Var&);
    wrpy_Varinfo* (*varinfo_create)(wreport::Varinfo);
    wrpy_Vartable* (*vartable_create)(const wreport::Vartable*);
};

}

#endif
