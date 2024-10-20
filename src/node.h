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

#include "util.h"

#ifndef TXC_NODE_DEPTH_TYPE
#define TXC_NODE_DEPTH_TYPE uint_fast8_t
#endif /* TXC_NODE_DEPTH_TYPE */
#ifndef TXC_NODE_DEPTH_MAX
#define TXC_NODE_DEPTH_MAX UINT_FAST8_MAX
#endif /* TXC_NODE_DEPTH_MAX */

/* CONSTANTS */

extern const struct txc_node TXC_NAN_ERROR_ALLOC;
extern const struct txc_node TXC_NAN_ERROR_INVALID_NODE_TYPE;
extern const struct txc_node TXC_NAN_ERROR_OVERFLOW;
extern const struct txc_node TXC_NAN_ERROR_NYI;
extern const struct txc_node TXC_NAN_UNSPECIFIED;
extern const struct txc_node TXC_NAN_ZERO_DIVISION;

/* VALID */

extern bool txc_node_test_valid(const struct txc_node *const node, const bool recursive);

/* GETTER AND SETTER */

extern size_t txc_node_get_gc_i(const struct txc_node *const node);

extern bool txc_node_get_read_only(const struct txc_node *const node);

extern struct txc_node *txc_node_set_gc_i(struct txc_node *const node, size_t gc_i);

/* MEMORY */

extern const struct txc_int *txc_node_to_int(const struct txc_node *const node);

extern const struct txc_node *txc_node_create(txc_mem_gc *const gc, const struct txc_node *const *const children, const union impl impl, const size_t children_amount, const enum txc_node_type type);

extern const struct txc_node *txc_node_create_nan(txc_mem_gc *const gc, const char *const reason);

extern const struct txc_node *txc_node_create_un_op(txc_mem_gc *const gc, const enum txc_node_type type, const struct txc_node *const operand);

extern const struct txc_node *txc_node_create_bin_op(txc_mem_gc *const gc, const enum txc_node_type type, const struct txc_node *const operand_1, const struct txc_node *const operand_2);

// extern const struct txc_node *txc_node_copy_read(const struct txc_node *const from);

// extern const struct txc_node *txc_node_copy_write(const struct txc_node *const from);

extern void txc_node_free(const struct txc_node *const node, const TXC_NODE_DEPTH_TYPE depth);

/* NODE */

extern const struct txc_node *txc_node_simplify(txc_mem_gc *const gc, const struct txc_node *const node);

/* PRINT */

extern char *txc_node_to_str(const struct txc_node *const node);

extern void txc_node_print(const struct txc_node *const node);

extern void txc_node_print_if_debug(const struct txc_node *const node);

extern void txc_node_simplify_and_print(txc_mem_gc *const gc, const struct txc_node *const node);

#endif /* TXC_NODE */
