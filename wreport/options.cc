#include "options.h"

namespace wreport {
namespace options {

thread_local bool var_silent_domain_errors = false;
thread_local bool var_clamp_domain_errors = false;

DomainErrorHook::~DomainErrorHook()
{
}

thread_local DomainErrorHook* var_hook_domain_errors = nullptr;

}
}
