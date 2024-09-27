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

#ifndef TXC_INTEGERS
#define TXC_INTEGERS

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "node.h"

/* DEFINITIONS */

typedef struct txc_int txc_int;

/* MEMORY */

extern txc_node *txc_int_create_int(const char *const str, size_t len, uint_fast8_t base);

extern txc_int *txc_int_create_zero(void);

extern txc_int *txc_int_create_one(void);

extern txc_int *txc_int_copy_read(const txc_int *const from);

extern txc_int *txc_int_copy_write(const txc_int *const from);

extern void txc_int_free(txc_int *const integer);

/* CONVERT */

extern txc_node *txc_int_to_node(txc_int *const integer);

/* INT */

bool txc_int_is_pos_one(const txc_int *test);

bool txc_int_is_zero(const txc_int *test);

bool txc_int_is_neg_one(const txc_int *test);

bool txc_int_is_neg(const txc_int *test);

extern int_fast8_t txc_int_cmp_abs(const txc_int *const a, const txc_int *const b);

extern int_fast8_t txc_int_cmp(const txc_int *const a, const txc_int *const b);

extern txc_int *txc_int_neg(const txc_int *const integer);

extern txc_int *txc_int_add(const txc_int *const *const summands, const size_t len);

extern txc_int *txc_int_mul(const txc_int *const *const factors, const size_t len);

extern txc_int *txc_int_mul(txc_int *const *const factors, const size_t len);

/* PRINT */

extern const char *txc_int_to_str(txc_int *const integer);

#endif /* TXC_INTEGERS */
