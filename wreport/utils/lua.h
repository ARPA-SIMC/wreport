/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

// Copyright (C) 2014  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef WREPORT_UTILS_LUA_H
#define WREPORT_UTILS_LUA_H

#include "config.h"

#ifndef HAVE_LUA
#ifdef WREPORT_LUA_REQUIRED
#error This source requires Lua to compile
#endif
#else
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

namespace wreport {
namespace lua {

template <typename T>
void push_object(lua_State* L, T* obj, const char* class_name,
                 const luaL_Reg* lib)
{
    // The object we create is a userdata that holds a pointer to obj
    T** s = (T**)lua_newuserdata(L, sizeof(T*));
    *s    = obj;

    // Set the metatable for the userdata
    if (luaL_newmetatable(L, class_name))
    {
        // If the metatable wasn't previously created, create it now
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2); /* pushes the metatable */
        lua_settable(L, -3);  /* metatable.__index = metatable */

        // Load normal methods
#if LUA_VERSION_NUM >= 502
        luaL_setfuncs(L, lib, 0);
#else
        luaL_register(L, NULL, lib);
#endif
    }

    lua_setmetatable(L, -2);
}

} // namespace lua
} // namespace wreport

#endif

#endif
