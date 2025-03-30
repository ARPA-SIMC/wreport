#ifndef WREPORT_ERROR_H
#define WREPORT_ERROR_H

#include <stdexcept>
#include <string>

/** @file
 * wreport exceptions.
 *
 * All wreport exceptions are derived from wreport::error, which is in turn
 * derived from std::exception.
 *
 * All wreport exceptions also have an exception specific error code, which
 * makes it easy to turn a caught exception into an errno-style error code, when
 * providing C or Fortran bindings.
 */

namespace wreport {

/// C-style error codes used by exceptions
enum ErrorCode {
    /// No error
    WR_ERR_NONE          = 0,
    /// Item not found
    WR_ERR_NOTFOUND      = 1,
    /// Wrong variable type
    WR_ERR_TYPE          = 2,
    /// Cannot allocate memory
    WR_ERR_ALLOC         = 3,
    /// ODBC error
    WR_ERR_ODBC          = 4,
    /// Handle management error
    WR_ERR_HANDLES       = 5,
    /// Buffer is too short to fit data
    WR_ERR_TOOLONG       = 6,
    /// Error reported by the system
    WR_ERR_SYSTEM        = 7,
    /// Consistency check failed
    WR_ERR_CONSISTENCY   = 8,
    /// Parse error
    WR_ERR_PARSE         = 9,
    /// Write error
    WR_ERR_WRITE         = 10,
    /// Regular expression error
    WR_ERR_REGEX         = 11,
    /// Feature not implemented
    WR_ERR_UNIMPLEMENTED = 12,
    /// Value outside acceptable domain
    WR_ERR_DOMAIN        = 13
};

/**
 * Tell the compiler that a function always throws and expects printf-style
 * arguments
 */
#define WREPORT_THROWF_ATTRS(a, b)                                             \
    __attribute__((noreturn, format(printf, a, b)))

/// Base class for DB-All.e exceptions
class error : public std::exception
{
public:
    /**
     * Exception-specific error code
     *
     * This is useful to map C++ exceptions to C or Fortran error codes
     */
    virtual ErrorCode code() const noexcept = 0;

    /// Error message
    const char* what() const noexcept override = 0;

    /// String description for an error code
    static const char* strerror(ErrorCode code);
};

/// Reports that memory allocation has failed.
class error_alloc : public error
{
public:
    /// error message returned by what()
    const char* msg;

    /**
     * @param msg
     *    error message. It is a plain const char* in this case in order to
     *    keep things as simple as possible in case we really are very short of
     *    memory.
     */
    error_alloc(const char* msg) : msg(msg) {}
    ~error_alloc() {}

    ErrorCode code() const noexcept override { return WR_ERR_ALLOC; }

    /// Throw the exception, building the message printf-style
    const char* what() const noexcept override { return msg; }
};

namespace errors {
template <ErrorCode ERROR_CODE> class StringBase : public error
{
public:
    /// error message returned by what()
    std::string msg;

    /// @param msg error message
    StringBase(const std::string& msg) noexcept : msg(msg) {}

    ErrorCode code() const noexcept override { return ERROR_CODE; }

    const char* what() const noexcept override { return msg.c_str(); }
};
} // namespace errors

/// Reports that a search-like function could not find what was requested.
class error_notfound : public errors::StringBase<WR_ERR_NOTFOUND>
{
public:
    using StringBase::StringBase;

    /// Throw the exception, building the message printf-style
    static void throwf(const char* fmt, ...) WREPORT_THROWF_ATTRS(1, 2);
};

/**
 * For functions handling data with multiple types, reports a mismatch
 * between the type requested and the type found.
 */
class error_type : public errors::StringBase<WR_ERR_TYPE>
{
public:
    using StringBase::StringBase;

    /// Throw the exception, building the message printf-style
    static void throwf(const char* fmt, ...) WREPORT_THROWF_ATTRS(1, 2);
};

/**
 * For functions working with handles, reports a problem with handling handles,
 * such as impossibility to allocate a new one, or an invalid handle being
 * passed to the function.
 */
class error_handles : public errors::StringBase<WR_ERR_HANDLES>
{
public:
    using StringBase::StringBase;

    /// Throw the exception, building the message printf-style
    static void throwf(const char* fmt, ...) WREPORT_THROWF_ATTRS(1, 2);
};

/// Report an error with a buffer being to short for the data it needs to fit.
class error_toolong : public errors::StringBase<WR_ERR_TOOLONG>
{
public:
    using StringBase::StringBase;

    /// Throw the exception, building the message printf-style
    static void throwf(const char* fmt, ...) WREPORT_THROWF_ATTRS(1, 2);
};

/**
 * Report a system error message.  The message description will be looked up
 * using the current value of errno.
 */
class error_system : public errors::StringBase<WR_ERR_SYSTEM>
{
public:
    /**
     * Create an exception taking further information from errno.
     *
     * @param msg error message
     */
    error_system(const std::string& msg);

    /**
     * Create an exception taking further information from an explicit errno
     * value.
     *
     * @param msg error message
     * @param errno_val explicit errno value
     */
    error_system(const std::string& msg, int errno_val);

    /// Throw the exception, building the message printf-style
    static void throwf(const char* fmt, ...) WREPORT_THROWF_ATTRS(1, 2);
};

/// Report an error when a consistency check failed.
class error_consistency : public errors::StringBase<WR_ERR_CONSISTENCY>
{
public:
    using StringBase::StringBase;

    /// Throw the exception, building the message printf-style
    static void throwf(const char* fmt, ...) WREPORT_THROWF_ATTRS(1, 2);
};

/// Report an error when parsing informations.
class error_parse : public errors::StringBase<WR_ERR_PARSE>
{
public:
    using StringBase::StringBase;

    /**
     * @param file
     *   The file that is being parsed
     * @param line
     *   The line of the file where the problem has been found
     * @param msg
     *   The error message
     */
    error_parse(const char* file, int line, const std::string& msg);

    /// Throw the exception, building the message printf-style
    static void throwf(const char* file, int line, const char* fmt, ...)
        WREPORT_THROWF_ATTRS(3, 4);
};

/// Report an error while handling regular expressions
class error_regexp : public errors::StringBase<WR_ERR_REGEX>
{
public:
    /**
     * @param code
     *   The error code returned by the regular expression functions.
     * @param re
     *   The pointer to the regex_t structure that was being used when the error
     *   occurred.
     * @param msg
     *   The error message
     */
    error_regexp(int code, void* re, const std::string& msg);

    /// Throw the exception, building the message printf-style
    static void throwf(int code, void* re, const char* fmt, ...)
        WREPORT_THROWF_ATTRS(3, 4);
};

/// Reports that a feature is still not implemented.
class error_unimplemented : public errors::StringBase<WR_ERR_UNIMPLEMENTED>
{
public:
    using StringBase::StringBase;

    /// Throw the exception, building the message printf-style
    static void throwf(const char* fmt, ...) WREPORT_THROWF_ATTRS(1, 2);
};

/// Report that a parameter is outside the acceptable domain
class error_domain : public errors::StringBase<WR_ERR_DOMAIN>
{
public:
    using StringBase::StringBase;

    /// Throw the exception, building the message printf-style
    static void throwf(const char* fmt, ...) WREPORT_THROWF_ATTRS(1, 2);
};

} // namespace wreport
#endif
