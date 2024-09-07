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

/* TYPEDEFS */

typedef struct txc_num txc_num;

/* SINGLETONS */

extern const txc_num TXC_NAN_ERROR_ALLOC;
extern const txc_num TXC_NAN_ERROR_NYI;
extern const txc_num TXC_NAN_ERROR_INVALID_NUM_TYPE;
extern const txc_num TXC_NAN_UNSPECIFIED;

extern const txc_num TXC_NIL_UNSPECIFIED;

extern const txc_num TXC_ZERO_ZERO;

/* CREATE, COPY AND FREE */

extern txc_num *txc_create_nan(const char *const reason);

extern txc_num *txc_create_nil(const char *const reason);

extern txc_num *txc_create_natural_num_or_zero(const char *const str, size_t len);

extern txc_num *txc_copy_num(txc_num *const from);

extern void txc_free_num(txc_num *const num);

/* NUM */

extern txc_num *txc_num_add(txc_num *const *const summands, const size_t len);

extern const char *txc_num_to_str(txc_num *const num);

#endif /* TXC_NUMBERS */
