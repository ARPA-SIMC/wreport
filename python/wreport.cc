#include "common.h"
#include "config.h"
#include "utils/methods.h"
#include "utils/values.h"
#include "var.h"
#include "varinfo.h"
#include "vartable.h"
#include "wreport/conv.h"

using namespace std;
using namespace wreport;
using namespace wreport::python;

namespace {

struct convert_units : public MethKwargs<convert_units, PyObject>
{
    constexpr static const char* name = "convert_units";
    constexpr static const char* signature =
        "from_unit: str, to_unit: str, value: float";
    constexpr static const char* returns = "float";
    constexpr static const char* summary =
        "convert a value from a unit to another, as understood by wreport";
    constexpr static const char* doc = nullptr;

    static PyObject* run(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = {"from_unit", "to_unit", "value",
                                       nullptr};
        const char* from_unit       = nullptr;
        const char* to_unit         = nullptr;
        double value;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "ssd",
                                         const_cast<char**>(kwlist), &from_unit,
                                         &to_unit, &value))
            return nullptr;

        try
        {
            return to_python(wreport::convert_units(from_unit, to_unit, value));
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

Methods<convert_units> methods;

} // namespace

extern "C" {

static PyModuleDef wreport_module = {
    PyModuleDef_HEAD_INIT,
    "_wreport",               /* m_name */
    "wreport Python library", /* m_doc */
    -1,                       /* m_size */
    methods.as_py(),          /* m_methods */
    NULL,                     /* m_reload */
    NULL,                     /* m_traverse */
    NULL,                     /* m_clear */
    NULL,                     /* m_free */

};

PyMODINIT_FUNC PyInit__wreport(void);

PyMODINIT_FUNC PyInit__wreport(void)
{
    static wrpy_c_api c_api;

    try
    {
        memset(&c_api, 0, sizeof(wrpy_c_api));
        c_api.version_major = 1;
        c_api.version_minor = 1;

        pyo_unique_ptr m(throw_ifnull(PyModule_Create(&wreport_module)));
        PyModule_AddStringConstant(m, "__version__", PACKAGE_VERSION);

        register_varinfo(m, c_api);
        register_vartable(m, c_api);
        register_var(m, c_api);

        // Create a Capsule containing the API struct's address
        pyo_unique_ptr c_api_object(throw_ifnull(
            PyCapsule_New((void*)&c_api, "_wreport._C_API", nullptr)));
        int res = PyModule_AddObject(m, "_C_API", c_api_object.release());
        if (res)
            return nullptr;

        return m.release();
    }
    WREPORT_CATCH_RETURN_PYO;
}
}
