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

#ifndef TXC_NODE
#define TXC_NODE

#include "common.h"

#include <stdbool.h>

/* DEFINITIONS */

enum txc_node_type {
    TXC_NAN,
    TXC_INT,
    TXC_NEG,
    TXC_ADD,
    TXC_MUL,
    TXC_FRAC
};

union impl {
    char *reason;
    struct txc_int *integer;
};

typedef struct txc_node txc_node;

/* CONSTANTS */

extern const txc_node TXC_NAN_ERROR_ALLOC;
extern const txc_node TXC_NAN_ERROR_INVALID_NODE_TYPE;
extern const txc_node TXC_NAN_ERROR_OVERFLOW;
extern const txc_node TXC_NAN_ERROR_NYI;
extern const txc_node TXC_NAN_ERROR_ZERO_DIVISION;
extern const txc_node TXC_NAN_UNSPECIFIED;

/* VALID */

extern bool txc_node_test_valid(const txc_node *const node, const bool recursive);

/* MEMORY */

extern struct txc_int *txc_node_to_int(txc_node *const node);

extern txc_node *txc_node_create(txc_node *const *const children, const union impl impl, const size_t children_amount, const enum txc_node_type type);

extern txc_node *txc_node_create_nan(const char *const reason);

extern txc_node *txc_node_create_un_op(const enum txc_node_type type, txc_node *const operand);

extern txc_node *txc_node_create_bin_op(const enum txc_node_type type, txc_node *const operand_1, txc_node *const operand_2);

extern void txc_node_free(const txc_node *const node);

/* NODE */

extern txc_node *txc_node_simplify(const txc_node *const node);

/* PRINT */

extern char *txc_node_to_str(const txc_node *const node);

extern void txc_node_print(const txc_node *const node);

extern void txc_node_print_if_debug(const txc_node *const node);

extern void txc_node_simplify_and_print(const txc_node *const node);

#endif /* TXC_NODE */
