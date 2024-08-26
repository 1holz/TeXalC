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
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

char *txc_strdup(const char *const str)
{
    assert(str != NULL);
    size_t len = strlen(str);
    char *copy = malloc(len + 1);
    if (copy == NULL)
        return NULL;
    memcpy(copy, str, len + 1);
    return copy;
}

char *txc_strrev(char *const str)
{
    assert(str != NULL);
    char *start = str;
    char *end = start + strlen(start) - 1;
    while (start < end)
    {
        *start ^= *end;
        *end ^= *start;
        *start ^= *end;
        start++;
        end--;
    }
    return str;
}
