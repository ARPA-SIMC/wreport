/*
 * info - print library configuration
 *
 * Copyright (C) 2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include <cstdlib>

// Print information about the library
void do_info()
{
    printf("Tables search paths (tried in order):\n");
    printf("Extra tables directory: %s (env var WREPORT_EXTRA_TABLES)\n", getenv("WREPORT_EXTRA_TABLES"));
    printf("System tables directory: %s (env var WREPORT_TABLES)\n", getenv("WREPORT_TABLES"));
    printf("Compiled-in default tables directory: %s\n", TABLE_DIR);
}
