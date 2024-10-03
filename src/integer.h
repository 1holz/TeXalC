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

#ifndef TXC_INTEGER
#define TXC_INTEGER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "node.h"

/* DEFINITIONS */

typedef struct txc_int txc_int;

// TODO const?
struct txc_txc_int_tuple
{
    txc_int *a;
    txc_int *b;
};

/* ASSERTS */

extern void txc_int_assert_valid(const txc_int *const integer);

/* MEMORY */

extern const txc_node *txc_int_to_node(const txc_int *const integer);

extern const txc_node *txc_int_create_int_node(const char *const str, size_t len, const uint_fast8_t base);

extern txc_int *txc_int_create_zero(void);

extern txc_int *txc_int_create_one(void);

extern txc_int *txc_int_copy(const txc_int *const from);

extern void txc_int_free(const txc_int *const integer);

/* INTEGER */

bool txc_int_is_pos_one(const txc_int *const test);

bool txc_int_is_zero(const txc_int *const test);

bool txc_int_is_neg_one(const txc_int *const test);

bool txc_int_is_neg(const txc_int *const test);

extern int_fast8_t txc_int_cmp_abs(const txc_int *const a, const txc_int *const b);

extern int_fast8_t txc_int_cmp(const txc_int *const a, const txc_int *const b);

extern txc_int *txc_int_neg(txc_int *const integer);

extern txc_int *txc_int_add(const txc_int *const *const summands, const size_t len);

extern txc_int *txc_int_mul(const txc_int *const *const factors, const size_t len);

extern const txc_int *txc_int_gcd(const txc_int *const aa, const txc_int *const bb);

extern struct txc_txc_int_tuple *txc_int_div_mod(const txc_int *const dividend, const txc_int *const divisor);

extern const struct txc_int *txc_int_div(const struct txc_int *const dividend, const struct txc_int *const divisor);

extern const struct txc_int *txc_int_mod(const struct txc_int *const dividend, const struct txc_int *const divisor);

/* PRINT */

extern char *txc_int_to_str(const txc_int *const integer);

#endif /* TXC_INTEGER */
