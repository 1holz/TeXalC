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

#ifndef TXC_NUMBERS
#define TXC_NUMBERS

#include <stdbool.h>
#include <stddef.h>

#define TXC_NUM_ARRAY_TYPE unsigned char

/* TYPEDEFS */

typedef enum txc_num_type
{
    TXC_NAN,
    TXC_NIL,
    TXC_ZERO,
    TXC_NATURAL_NUM
} txc_num_type;

typedef struct txc_num_array
{
    TXC_NUM_ARRAY_TYPE *data;
    size_t size;
    size_t used;
} txc_num_array;

typedef struct txc_num
{
    union
    {
        txc_num_array *natural_num;
    } impl;
    char *str;
    txc_num_type type;
    bool singleton;
} txc_num;

/* SINGLETONS */

extern const txc_num TXC_NAN_ERROR_ALLOC;
extern const txc_num TXC_NAN_ERROR_NYI;
extern const txc_num TXC_NAN_UNSPECIFIED;

extern const txc_num TXC_NIL_UNSPECIFIED;

extern const txc_num TXC_ZERO_ZERO;

/* CREATE */

extern txc_num *txc_create_nan(char *const reason);

extern txc_num *txc_create_nil(char *const reason);

extern txc_num *txc_create_natural_num_or_zero(char *const str, size_t len);

/* NUM */

extern const char *txc_num_to_str(txc_num *const num);

#endif /* TXC_NUMBERS */
