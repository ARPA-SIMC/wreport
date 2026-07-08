#include "options.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>

namespace wreport {
namespace options {

thread_local bool var_silent_domain_errors = false;
thread_local bool var_clamp_domain_errors  = false;

DomainErrorHook::~DomainErrorHook() {}

thread_local DomainErrorHook* var_hook_domain_errors = nullptr;

/*
 * MasterTableVersionOverride
 */

const int MasterTableVersionOverride::NONE;
const int MasterTableVersionOverride::NEWEST;

MasterTableVersionOverride::MasterTableVersionOverride()
{
    const char* env = getenv("WREPORT_MASTER_TABLE_VERSION");
    if (env == nullptr)
        value = NONE;
    else if (strcmp(env, "newest") == 0)
        value = NEWEST;
    else
    {
        errno       = 0;
        long lvalue = strtol(env, nullptr, 10);
        if (errno != 0)
            value = NONE;
        else
            value = lvalue;
    }
}

MasterTableVersionOverride::MasterTableVersionOverride(int value) : value(value)
{
}

thread_local MasterTableVersionOverride var_master_table_version_override;

} // namespace options
} // namespace wreport
