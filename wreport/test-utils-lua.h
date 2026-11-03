/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

// Copyright (C) 2008--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef WREPORT_TEST_UTILS_LUA_H
#define WREPORT_TEST_UTILS_LUA_H

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#include <iosfwd>
#include <string>

namespace wreport {
namespace tests {

struct Lua
{
    lua_State* L;
    std::string m_filename;

    Lua(const std::string& src = std::string());
    ~Lua();

    /// Load the test code from the given file
    void loadFile(const std::string& fname);

    /// Load the test code from the given string containing Lua source code
    void loadString(const std::string& buf);

    /// Runs the parsed code to let it define the 'test' function we are going
    /// to use
    void create_lua_object();

    /// Send Lua's print output to an ostream
    void captureOutput(std::ostream& buf);

    /// Run the 'test' function and return its result, as a string
    std::string run();
};

} // namespace tests
} // namespace wreport

#endif
