#ifndef WREPORT_PYTHON_COMMON_H
#define WREPORT_PYTHON_COMMON_H

#include <wreport/error.h>
#include <wreport/varinfo.h>
#include "utils/core.h"

namespace wreport {
namespace python {

/**
 * Return a python string representing a varcode
 */
PyObject* wrpy_varcode_format(wreport::Varcode code);

/// Given a wreport exception, set the Python error indicator appropriately.
void set_wreport_exception(const wreport::error& e);

/**
 * Given a wreport exception, set the Python error indicator appropriately.
 *
 * @retval
 *   Always returns NULL, so one can do:
 *   try {
 *     // ...code...
 *   } catch (wreport::error& e) {
 *     return raise_wreport_exception(e);
 *   }
 */
PyObject* raise_wreport_exception(const wreport::error& e);

/// Given a generic exception, set the Python error indicator appropriately.
void set_std_exception(const std::exception& e);

/**
 * Given a generic exception, set the Python error indicator appropriately.
 *
 * @retval
 *   Always returns NULL, so one can do:
 *   try {
 *     // ...code...
 *   } catch (std::exception& e) {
 *     return raise_std_exception(e);
 *   }
 */
PyObject* raise_std_exception(const std::exception& e);

#define WREPORT_CATCH_RETURN_PYO \
      catch (PythonException&) { \
        return nullptr; \
    } catch (wreport::error& e) { \
        set_wreport_exception(e); return nullptr; \
    } catch (std::exception& se) { \
        set_std_exception(se); return nullptr; \
    }

#define WREPORT_CATCH_RETURN_INT \
      catch (PythonException&) { \
        return -1; \
    } catch (wreport::error& e) { \
        set_wreport_exception(e); return -1; \
    } catch (std::exception& se) { \
        set_std_exception(se); return -1; \
    }

#define WREPORT_CATCH_RETHROW_PYTHON \
      catch (PythonException&) { \
        throw; \
    } catch (wreport::error& e) { \
        set_wreport_exception(e); throw PythonException(); \
    } catch (std::exception& se) { \
        set_std_exception(se); throw PythonException(); \
    }

/// Call repr() on \a o, and return the result in \a out
std::string object_repr(PyObject* o);

/**
 * call o.fileno() and return its result.
 *
 * In case of AttributeError and IOError (parent of UnsupportedOperation, not
 * available from C), it clear the error indicator.
 *
 * Returns -1 if fileno() was not available or some other exception happened.
 * Use PyErr_Occurred to tell between the two.
 */
int file_get_fileno(PyObject* o);

/**
 * call o.data() and return its result, both as a PyObject and as a buffer.
 *
 * The data returned in buf and len will be valid as long as the returned
 * object stays valid.
 */
PyObject* file_get_data(PyObject* o, char*&buf, Py_ssize_t& len);

}
}
#endif
