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
extern PyTypeObject* wrpy_RawBufrIterator_Type;
#define wrpy_RawBufrIterator_Check(ob)                                         \
    (Py_TYPE(ob) == wrpy_RawBufrIterator_Type ||                               \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_RawBufrIterator_Type))

/// wreport.RawCrexIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_RawCrexIterator;
extern PyTypeObject* wrpy_RawCrexIterator_Type;
#define wrpy_RawCrexIterator_Check(ob)                                         \
    (Py_TYPE(ob) == wrpy_RawCrexIterator_Type ||                               \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_RawCrexIterator_Type))

/// wreport.BufrIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_BufrIterator;
extern PyTypeObject* wrpy_BufrIterator_Type;
#define wrpy_BufrIterator_Check(ob)                                            \
    (Py_TYPE(ob) == wrpy_BufrIterator_Type ||                                  \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_BufrIterator_Type))

/// wreport.CrexIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_CrexIterator;
extern PyTypeObject* wrpy_CrexIterator_Type;
#define wrpy_CrexIterator_Check(ob)                                            \
    (Py_TYPE(ob) == wrpy_CrexIterator_Type ||                                  \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_CrexIterator_Type))

/// wreport.BufrHeaderIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_BufrHeaderIterator;
extern PyTypeObject* wrpy_BufrHeaderIterator_Type;
#define wrpy_BufrHeaderIterator_Check(ob)                                      \
    (Py_TYPE(ob) == wrpy_BufrHeaderIterator_Type ||                            \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_BufrHeaderIterator_Type))

/// wreport.CrexHeaderIterator python object
typedef struct
{
    PyObject_HEAD wreport::InputFile* file;
} wrpy_CrexHeaderIterator;
extern PyTypeObject* wrpy_CrexHeaderIterator_Type;
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

/// wreport.BulletinSubsetIterator python object
typedef struct
{
    PyObject_HEAD wrpy_Bulletin* bulletin;
    unsigned subset;
} wrpy_BulletinSubsetIterator;
extern PyTypeObject* wrpy_BulletinSubsetIterator_Type;
#define wrpy_BulletinSubsetIterator_Check(ob)                                  \
    (Py_TYPE(ob) == wrpy_BulletinSubsetIterator_Type ||                        \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_BulletinSubsetIterator_Type))

/// wreport.Subset python object
typedef struct
{
    PyObject_HEAD wrpy_Bulletin* bulletin;
    unsigned subset;
} wrpy_Subset;
extern PyTypeObject* wrpy_Subset_Type;
#define wrpy_Subset_Check(ob)                                                  \
    (Py_TYPE(ob) == wrpy_Subset_Type ||                                        \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_Subset_Type))

/// wreport.SubsetVarIterator python object
typedef struct
{
    PyObject_HEAD wrpy_Subset* subset;
    unsigned var;
} wrpy_SubsetVarIterator;
extern PyTypeObject* wrpy_SubsetVarIterator_Type;
#define wrpy_SubsetVarIterator_Check(ob)                                       \
    (Py_TYPE(ob) == wrpy_SubsetVarIterator_Type ||                             \
     PyType_IsSubtype(Py_TYPE(ob), wrpy_SubsetVarIterator_Type))
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
