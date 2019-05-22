#include <Python.h>
#include "common.h"
#include "vartable.h"
#include "varinfo.h"
#include "var.h"
#include "config.h"

using namespace std;
using namespace wreport;
using namespace wreport::python;

extern "C" {

static PyMethodDef wreport_methods[] = {
    { NULL }
};

static PyModuleDef wreport_module = {
    PyModuleDef_HEAD_INIT,
    "_wreport",       /* m_name */
    "wreport Python library",  /* m_doc */
    -1,             /* m_size */
    wreport_methods, /* m_methods */
    NULL,           /* m_reload */
    NULL,           /* m_traverse */
    NULL,           /* m_clear */
    NULL,           /* m_free */

};

PyMODINIT_FUNC PyInit__wreport(void)
{
    using namespace wreport::python;

static wrpy_c_api c_api;
    memset(&c_api, 0, sizeof(wrpy_c_api));
    c_api.version_major = 1;
    c_api.version_minor = 0;

    PyObject* m;

    m = PyModule_Create(&wreport_module);

    if (register_varinfo(m, c_api))
        return nullptr;
    if (register_vartable(m, c_api))
        return nullptr;
    if (register_var(m, c_api))
        return nullptr;

    // Create a Capsule containing the API struct's address
    pyo_unique_ptr c_api_object(PyCapsule_New((void *)&c_api, "_wreport._C_API", nullptr));
    if (!c_api_object)
        return nullptr;

    int res = PyModule_AddObject(m, "_C_API", c_api_object.release());
    if (res)
        return nullptr;

    return m;
}

}
