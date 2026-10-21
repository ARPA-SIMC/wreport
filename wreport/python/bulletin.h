#ifndef WREPORT_PYTHON_BULLETIN_H
#define WREPORT_PYTHON_BULLETIN_H

#include "utils/core.h"
#include <wreport/bulletin.h>
#include <wreport/python.h>
#include <wreport/python/files.h>

extern "C" {

/// wreport.RawBufrIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_RawBufrIterator;

/// wreport.RawBufrIterator python type
extern PyTypeObject* wrpy_RawBufrIterator_Type;

/// Check if an object is of wreport.RawBufrIterator type or subtype
#define wrpy_RawBufrIterator_Check(ob)                                         \
    (Py_TYPE(ob) == wrpy_RawBufrIterator_Type ||                               \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_RawBufrIterator_Type))

/// wreport.RawCrexIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_RawCrexIterator;

/// wreport.RawCrexIterator python type
extern PyTypeObject* wrpy_RawCrexIterator_Type;

/// Check if an object is of wreport.RawCrexIterator type or subtype
#define wrpy_RawCrexIterator_Check(ob)                                         \
    (Py_TYPE(ob) == wrpy_RawCrexIterator_Type ||                               \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_RawCrexIterator_Type))

/// wreport.BufrIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_BufrIterator;

/// wreport.BufrIterator python type
extern PyTypeObject* wrpy_BufrIterator_Type;

/// Check if an object is of wreport.BufrIterator type or subtype
#define wrpy_BufrIterator_Check(ob)                                            \
    (Py_TYPE(ob) == wrpy_BufrIterator_Type ||                                  \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_BufrIterator_Type))

/// wreport.CrexIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_CrexIterator;

/// wreport.CrexIterator python type
extern PyTypeObject* wrpy_CrexIterator_Type;

/// Check if an object is of wreport.CrexIterator type or subtype
#define wrpy_CrexIterator_Check(ob)                                            \
    (Py_TYPE(ob) == wrpy_CrexIterator_Type ||                                  \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_CrexIterator_Type))

/// wreport.BufrHeaderIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_BufrHeaderIterator;

/// wreport.BufrHeaderIterator python type
extern PyTypeObject* wrpy_BufrHeaderIterator_Type;

/// Check if an object is of wreport.BufrHeaderIterator type or subtype
#define wrpy_BufrHeaderIterator_Check(ob)                                      \
    (Py_TYPE(ob) == wrpy_BufrHeaderIterator_Type ||                            \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_BufrHeaderIterator_Type))

/// wreport.CrexHeaderIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_CrexHeaderIterator;

/// wreport.CrexHeaderIterator python type
extern PyTypeObject* wrpy_CrexHeaderIterator_Type;

/// Check if an object is of wreport.CrexHeaderIterator type or subtype
#define wrpy_CrexHeaderIterator_Check(ob)                                      \
    (Py_TYPE(ob) == wrpy_CrexHeaderIterator_Type ||                            \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_CrexHeaderIterator_Type))

/// wreport.Bulletin python object
typedef struct
{
    PyObject_HEAD wreport::Bulletin* bulletin;
} wrpy_Bulletin;

extern PyTypeObject* wrpy_Bulletin_Type;

#define wrpy_Bulletin_Check(ob)                                                \
    (Py_TYPE(ob) == wrpy_Bulletin_Type ||                                      \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_Bulletin_Type))
}

namespace wreport::python {

/**
 * Pass a bulletin to Python code.
 *
 * Python takes ownership of the bulletin object
 */
PyObject* bulletin_to_python(std::unique_ptr<wreport::Bulletin> bulletin);

void register_bulletin(PyObject* m, wrpy_c_api& c_c_api);

} // namespace wreport::python

#endif
