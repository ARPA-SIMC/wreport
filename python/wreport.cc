#include "common.h"
#include "vartable.h"
#include "varinfo.h"
#include "var.h"
#include "utils/methods.h"

using namespace std;
using namespace wreport;
using namespace wreport::python;

namespace {

Methods<> methods;

}

extern "C" {

static PyModuleDef wreport_module = {
    PyModuleDef_HEAD_INIT,
    "_wreport",       /* m_name */
    "wreport Python library",  /* m_doc */
    -1,             /* m_size */
    methods.as_py(), /* m_methods */
    NULL,           /* m_reload */
    NULL,           /* m_traverse */
    NULL,           /* m_clear */
    NULL,           /* m_free */

};

PyMODINIT_FUNC PyInit__wreport(void)
{
    static wrpy_c_api c_api;

    try {
        memset(&c_api, 0, sizeof(wrpy_c_api));
        c_api.version_major = 1;
        c_api.version_minor = 1;

        pyo_unique_ptr m(throw_ifnull(PyModule_Create(&wreport_module)));

        register_varinfo(m, c_api);
        register_vartable(m, c_api);
        register_var(m, c_api);

        // Create a Capsule containing the API struct's address
        pyo_unique_ptr c_api_object(throw_ifnull(PyCapsule_New((void *)&c_api, "_wreport._C_API", nullptr)));
        int res = PyModule_AddObject(m, "_C_API", c_api_object.release());
        if (res)
            return nullptr;

        return m.release();
    } WREPORT_CATCH_RETURN_PYO;
}

}
