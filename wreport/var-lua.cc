/*
 * wreport/var-lua - Lua bindings to wreport variables
 *
 * Copyright (C) 2010--2014  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "config.h"
#include "var.h"
#define WREPORT_LUA_REQUIRED
#include "utils/lua.h"

namespace wreport {

Var* Var::lua_check(lua_State* L, int idx)
{
    Var** v = (Var**)luaL_checkudata(L, idx, "dballe.var");
    return (v != NULL) ? *v : NULL;
}

const Var* Var::lua_const_check(lua_State* L, int idx)
{
    const Var** v = (const Var**)luaL_checkudata(L, idx, "dballe.var");
    return (v != NULL) ? *v : NULL;
}

static int dbalua_var_enqi(lua_State* L)
{
    const Var* var = Var::lua_const_check(L, 1);
    try
    {
        if (var->isset())
            lua_pushinteger(L, var->enqi());
        else
            lua_pushnil(L);
    }
    catch (std::exception& e)
    {
        lua_pushstring(L, e.what());
        lua_error(L);
    }
    return 1;
}

static int dbalua_var_enqd(lua_State* L)
{
    const Var* var = Var::lua_const_check(L, 1);
    try
    {
        if (var->isset())
            lua_pushnumber(L, var->enqd());
        else
            lua_pushnil(L);
    }
    catch (std::exception& e)
    {
        lua_pushstring(L, e.what());
        lua_error(L);
    }
    return 1;
}

static int dbalua_var_enqc(lua_State* L)
{
    const Var* var = Var::lua_const_check(L, 1);
    try
    {
        if (var->isset())
            lua_pushstring(L, var->enqc());
        else
            lua_pushnil(L);
    }
    catch (std::exception& e)
    {
        lua_pushstring(L, e.what());
        lua_error(L);
    }
    return 1;
}

static int dbalua_var_code(lua_State* L)
{
    static char fcodes[] = "BRCD";
    const Var* var       = Var::lua_const_check(L, 1);
    char buf[10];
    snprintf(buf, 10, "%c%02d%03d", fcodes[WR_VAR_F(var->code())],
             WR_VAR_X(var->code()), WR_VAR_Y(var->code()));
    lua_pushstring(L, buf);
    return 1;
}

static int dbalua_var_tostring(lua_State* L)
{
    const Var* var = Var::lua_const_check(L, 1);
    try
    {
        std::string formatted = var->format("(undef)");
        lua_pushlstring(L, formatted.data(), formatted.size());
    }
    catch (std::exception& e)
    {
        lua_pushstring(L, e.what());
        lua_error(L);
    }
    return 1;
}

static const struct luaL_Reg dbalua_var_lib[] = {
    {"code",       dbalua_var_code    },
    {"enqi",       dbalua_var_enqi    },
    {"enqd",       dbalua_var_enqd    },
    {"enqc",       dbalua_var_enqc    },
    {"__tostring", dbalua_var_tostring},
    {NULL,         NULL               }
};

void Var::lua_push(lua_State* L)
{
    lua::push_object(L, this, "dballe.var", dbalua_var_lib);
}

void Var::lua_push(lua_State* L) const
{
    lua::push_object(L, this, "dballe.var", dbalua_var_lib);
}

} // namespace wreport
