/*
 *     Copyright (C) 2024  Einholz
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published
 *     by the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 *
 *     You should have received a copy of the GNU Affero General Public License
 *     along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "common.h"

#include <stdio.h>

#include "memory.h"
#include "parser.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("This is TeXalC %u.%u.%u, a fancy calculator!\n", TXC_VERSION_MAJOR, TXC_VERSION_MINOR, TXC_VERSION_PATCH);
    int exit_code = yyparse(txc_mem_gc_init());
    printf("Exiting TeXalC\n");
    return exit_code;
}
