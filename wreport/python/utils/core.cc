#include "core.h"

namespace wreport::python {

const char* PythonException::what() const noexcept
{
    AcquireGIL gil;
    PyErr_FormatUnraisable("Python exception caught from C++");
    return "Python exception caught from C++";
}

} // namespace wreport::python
