/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef WREPORT_TERM_H
#define WREPORT_TERM_H

// Copyright (C) 2018  Enrico Zini <enrico@debian.org>
//
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdio>
#include <string>

namespace wreport::term {

struct Terminal
{
    static const unsigned black;
    static const unsigned red;
    static const unsigned green;
    static const unsigned yellow;
    static const unsigned blue;
    static const unsigned magenta;
    static const unsigned cyan;
    static const unsigned white;
    static const unsigned bright;

    FILE* out;
    bool isatty;

    struct Restore
    {
        Terminal& term;

        Restore(Terminal& term);
        ~Restore();
    };

    Terminal(FILE* out);

    Restore set_color(int fg, int bg);
    Restore set_color_fg(int col);
    Restore set_color_bg(int col);

    std::string color(int fg, int bg, const std::string& s);
    std::string color_fg(int col, const std::string& s);
    std::string color_bg(int col, const std::string& s);

    operator FILE*() { return out; }
};

} // namespace wreport::term

#endif
