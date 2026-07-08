/*
 * wreport/lua - Utilities used to interface with Lua
 *               This is not part of the wreport API!
 *
 * Copyright (C) 2014  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

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
