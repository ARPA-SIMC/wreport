#ifndef WREPORT_OPTIONS_H
#define WREPORT_OPTIONS_H

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
template<typename T>
struct LocalOverride
{
    T old_value;
    T& param;

    LocalOverride(T& param, const T& new_value)
        : old_value(param), param(param)
    {
        param = new_value;
    }
    ~LocalOverride()
    {
        param = old_value;
    }
};

template<typename T> static inline LocalOverride<T> local_override(T& param, const T& new_value)
{
    return LocalOverride<T>(param, new_value);
}

}
}

#endif
