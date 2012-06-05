/*
 * wreport/options - Library configuration
 *
 * Copyright (C) 2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#ifndef WREPORT_OPTIONS_H
#define WREPORT_OPTIONS_H

/** @file
 * @ingroup core
 * Implement wreport::Var, an encapsulation of a measured variable.
 */

namespace wreport {
namespace options {

/**
 * Whether domain errors on Var assignments raise exceptions.
 *
 * If true, domain errors on variable assignments are silent, and the target
 * variable gets set to undefined. If false (default), error_domain is raised.
 */
extern bool var_silent_domain_errors;

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

}
}

#endif
/* vim:set ts=4 sw=4: */
