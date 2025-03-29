#ifndef WREPORT_OPTIONS_H
#define WREPORT_OPTIONS_H

#include <cstdint>
#include <wreport/fwd.h>

/** @file
 *
 * Configuration variables to control configurable aspects of wreport's
 * behaviour.
 *
 * Variables are global and thread_local. They are global because they are
 * consulted in performance-critical code like Var::seti, and they are
 * thread_local so that a thread that changes its own configuration does not
 * affect the others.
 *
 * LocalOverride can be used to perform configuration changes for the duration
 * of a scope. Note that if while the override is active you pass control to an
 * unrelated part of the code which also uses wreport, the behaviour of that
 * code is also changed.
 */

/// Newly introduced options get a way for code to test for their existance
#define WREPORT_OPTIONS_HAS_VAR_CLAMP_DOMAIN_ERRORS
#define WREPORT_OPTIONS_HAS_VAR_HOOK_DOMAIN_ERRORS

namespace wreport {
namespace options {

/**
 * Whether domain errors on Var assignments raise exceptions.
 *
 * If true, domain errors on variable assignments are silent, and the target
 * variable gets set to undefined. If false (default), error_domain is raised.
 */
extern thread_local bool var_silent_domain_errors;

/**
 * Whether domain errors on Var assignments raise exceptions.
 *
 * If true, domain errors on variable assignments are silent, and the target
 * variable gets set to the minimum or maximum extreme value of the domain
 * corresponding to the direction the value is overflowing.
 */
extern thread_local bool var_clamp_domain_errors;

/**
 * Interface for a callback hook system to delegate handling domain errors to
 * the code using wreport.
 */
struct DomainErrorHook
{
    virtual ~DomainErrorHook();
    virtual void handle_domain_error_int(Var& var, int32_t val) = 0;
    virtual void handle_domain_error_double(Var& var, double val) = 0;
};

/**
 * If set, delegate handling domain errors to this object.
 *
 * Other than calling the hook functions, setting an out-of-domain value will
 * do nothing. The object will have responsibility for setting or unsetting the
 * Var as needed.
 */
extern thread_local DomainErrorHook* var_hook_domain_errors;


class MasterTableVersionOverride
{
    int value;

public:
    /// No override
    static const int NONE = 0;
    /// Use the latest available master table number
    static const int NEWEST = -1;

    /// Initialize from environment
    MasterTableVersionOverride();

    /// Initialize from a value
    // cppcheck-suppress noExplicitConstructor; This is intending to pose as an integer value
    MasterTableVersionOverride(int value);

    ~MasterTableVersionOverride() = default;
    MasterTableVersionOverride(const MasterTableVersionOverride&) = default;
    MasterTableVersionOverride(MasterTableVersionOverride&&) = default;
    MasterTableVersionOverride& operator=(const MasterTableVersionOverride&) = default;
    MasterTableVersionOverride& operator=(MasterTableVersionOverride&&) = default;

    /// Access the variable as an integer
    operator int() const { return value; }
};

/**
 * If set, ignore the master table number in BUFR and CREX messages and use the
 * one from this variable.
 */
extern thread_local MasterTableVersionOverride var_master_table_version_override;

/**
 * Temporarily override a variable while this object is in scope.
 *
 * Note that if the variable is global, then the override is temporally limited
 * to the scope, but it is seen by all the functions that reference the
 * variable functions.
 *
 * Example:
 * \code
 * {
 *     auto o = options::local_override(options::var_silent_domain_errors, true);
 *     var.setd(value)
 * }
 * \endcode
 */
template<typename T, typename T1 = T>
struct LocalOverride
{
    T old_value;
    T& param;

    LocalOverride(T& param, T1 new_value)
        : old_value(param), param(param)
    {
        param = new_value;
    }
    ~LocalOverride()
    {
        param = old_value;
    }
};

template<typename T, typename T1 = T> static inline LocalOverride<T> local_override(T& param, T1 new_value)
{
    return LocalOverride<T>(param, new_value);
}

}
}

#endif
