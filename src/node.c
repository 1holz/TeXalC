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

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "node.h"
#include "integer.h"
#include "memory.h"

#define TXC_NAN_REASON "\\text{NAN(%s)}"
#define TXC_NAN_REASON_ERROR_ALLOC "Could not allocate enough memory. Please see stderr for more information."
#define TXC_NAN_REASON_ERROR_INVALID_NODE_TYPE "Node type is invalid. Please see stderr for more information."
#define TXC_NAN_REASON_ERROR_NYI "Not yet implemented. Please see stderr for more information."
#define TXC_NAN_REASON_ERROR_OVERFLOW "Overflow occured. Please see stderr for more information."
#define TXC_NAN_REASON_UNSPECIFIED "unspecified"
#define TXC_NAN_REASON_ZERO_DIVISION "Divided by 0."

#define TXC_PRINT_FORMAT "= %s \\\\\n"
#define TXC_PRINT_FORMAT_NEWLINE "= %s \\\\\n\n"
#define TXC_PRINT_ERROR "\\text{PRINT(Unable to print. Please see stderr for more information.)}"

/* CONSTANTS */

const struct txc_node TXC_NAN_ERROR_ALLOC = { .impl.reason = TXC_NAN_REASON_ERROR_ALLOC,
                                              .children_amount = 0,
                                              .type = TXC_NAN,
                                              .read_only = true };
const struct txc_node TXC_NAN_ERROR_INVALID_NODE_TYPE = { .impl.reason = TXC_NAN_REASON_ERROR_INVALID_NODE_TYPE,
                                                          .children_amount = 0,
                                                          .type = TXC_NAN,
                                                          .read_only = true };
const struct txc_node TXC_NAN_ERROR_NYI = { .impl.reason = TXC_NAN_REASON_ERROR_NYI,
                                            .children_amount = 0,
                                            .type = TXC_NAN,
                                            .read_only = true };
const struct txc_node TXC_NAN_ERROR_OVERFLOW = { .impl.reason = TXC_NAN_REASON_ERROR_OVERFLOW,
                                                 .children_amount = 0,
                                                 .type = TXC_NAN,
                                                 .read_only = true };
const struct txc_node TXC_NAN_UNSPECIFIED = { .impl.reason = TXC_NAN_REASON_UNSPECIFIED,
                                              .children_amount = 0,
                                              .type = TXC_NAN,
                                              .read_only = true };
const struct txc_node TXC_NAN_ZERO_DIVISION = { .impl.reason = TXC_NAN_REASON_ZERO_DIVISION,
                                                .children_amount = 0,
                                                .type = TXC_NAN,
                                                .read_only = true };

/* VALID */

bool txc_node_test_valid(const struct txc_node *const node, const bool recursive)
{
    if (node == NULL) {
        TXC_ERROR_NULL("node");
        return false;
    }
    for (size_t i = 0; recursive && i < node->children_amount; i++) {
        if (!txc_node_test_valid(node->children[i], recursive))
            return false;
    }
    switch (node->type) {
    case TXC_INT:
        if (node->children_amount != 0) {
            TXC_ERROR_INVALID_CHILD_AMOUNT(node->type, node->children_amount);
            return false;
        }
        if (!txc_int_test_valid(node->impl.integer))
            return false;
        return true;
    case TXC_NEG:
        if (node->children_amount != 1) {
            TXC_ERROR_INVALID_CHILD_AMOUNT(node->type, node->children_amount);
            return false;
        }
        return true;
    case TXC_FRAC:
        if (node->children_amount < 1 || node->children_amount > 2) {
            TXC_ERROR_INVALID_CHILD_AMOUNT(node->type, node->children_amount);
            return false;
        }
        return true;
    case TXC_NAN: /* FALLTHROUGH */
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL:
        return true;
    default:
        TXC_ERROR_INVALID_NODE_TYPE(node->type);
        return false;
    }
}

/* GETTER AND SETTER */

size_t txc_node_get_gc_i(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    return node->gc_i;
}

bool txc_node_get_read_only(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    return node->read_only;
}

struct txc_node *txc_node_set_gc_i(struct txc_node *const node, size_t gc_i)
{
    assert(txc_node_test_valid(node, true));
    node->gc_i = gc_i;
    return node;
}

/* MEMORY */

/*
static const struct txc_node *bake(struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    return node;
}
*/

const txc_int *txc_node_to_int(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    return node->type == TXC_INT ? node->impl.integer : NULL;
}

const struct txc_node *txc_node_create(txc_mem_gc *gc, const struct txc_node *const *const children, const union impl impl, const size_t children_amount, const enum txc_node_type type)
{
    for (size_t i = 0; i < children_amount; i++)
        assert(txc_node_test_valid(children[i], true));
    struct txc_node *const node = malloc(sizeof *node + sizeof *node->children * children_amount);
    if (node == NULL) {
        TXC_ERROR_ALLOC(sizeof *node + sizeof *node->children * children_amount, "node");
        return &TXC_NAN_ERROR_ALLOC;
    }
    node->impl = impl;
    node->children_amount = children_amount;
    node->type = type;
    node->read_only = false;
    for (size_t i = 0; i < children_amount; i++)
        node->children[i] = children[i];
    return txc_mem_gc_bake(gc, node);
}

const struct txc_node *txc_node_create_nan(txc_mem_gc *gc, const char *const reason)
{
    if (reason == NULL)
        return &TXC_NAN_UNSPECIFIED;
    struct txc_node *nan = malloc(sizeof *nan);
    if (nan == NULL) {
        TXC_ERROR_ALLOC(sizeof *nan, "nan");
        return &TXC_NAN_ERROR_ALLOC;
    }
    nan->impl.reason = txc_strdup(reason);
    if (nan->impl.reason == NULL) {
        TXC_ERROR_ALLOC(strlen(reason) + 1, "nan reason");
        free(nan);
        return &TXC_NAN_ERROR_ALLOC;
    }
    nan->children_amount = 0;
    nan->type = TXC_NAN;
    nan->read_only = false;
    return txc_mem_gc_bake(gc, nan);
}

static const struct txc_node *init_op(txc_mem_gc *const gc, const enum txc_node_type type, const struct txc_node *const *const children, const size_t children_amount)
{
    for (size_t i = 0; i < children_amount; i++)
        assert(txc_node_test_valid(children[i], true));
    struct txc_node *const node = malloc(sizeof *node + sizeof *node->children * children_amount);
    if (node == NULL) {
        TXC_ERROR_ALLOC(sizeof *node, "operation node");
        return &TXC_NAN_ERROR_ALLOC;
    }
    node->children_amount = children_amount;
    node->type = type;
    node->read_only = false;
    for (size_t i = 0; i < children_amount; i++)
        node->children[i] = txc_mem_gc_copy(gc, children[i]);
    return txc_mem_gc_bake(gc, node);
}

const struct txc_node *txc_node_create_un_op(txc_mem_gc *const gc, const enum txc_node_type type, const struct txc_node *const operand)
{
    if (type != TXC_NEG && type != TXC_FRAC) {
        TXC_ERROR_INVALID_NODE_TYPE(type);
        return &TXC_NAN_ERROR_INVALID_NODE_TYPE;
    }
    assert(txc_node_test_valid(operand, true));
    const size_t arity = 1;
    const struct txc_node *const children[] = { operand };
    return init_op(gc, type, children, arity);
}

const struct txc_node *txc_node_create_bin_op(txc_mem_gc *const gc, const enum txc_node_type type, const struct txc_node *const operand_1, const struct txc_node *const operand_2)
{
    if (type != TXC_ADD && type != TXC_MUL && type != TXC_FRAC) {
        TXC_ERROR_INVALID_NODE_TYPE(type);
        return &TXC_NAN_ERROR_INVALID_NODE_TYPE;
    }
    assert(txc_node_test_valid(operand_1, true));
    assert(txc_node_test_valid(operand_2, true));
    const size_t arity = 2;
    const struct txc_node *const children[] = { operand_1, operand_2 };
    return init_op(gc, type, children, arity);
}

static struct txc_node *copy(txc_mem_gc *const gc, const struct txc_node *const from, const TXC_NODE_DEPTH_TYPE depth)
{
    assert(txc_node_test_valid(from, true));
    if (from->read_only)
        return (struct txc_node *)from;
    struct txc_node *cpy = malloc(sizeof *cpy + sizeof *cpy->children * from->children_amount);
    if (cpy == NULL) {
        TXC_ERROR_ALLOC(sizeof *cpy + sizeof *cpy->children * from->children_amount, "copy");
        return (struct txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    cpy->children_amount = from->children_amount;
    cpy->type = from->type;
    cpy->read_only = false;
    for (size_t i = 0; i < cpy->children_amount; i++) {
        if (depth == 0)
            cpy->children[i] = txc_mem_gc_copy(gc, from->children[i]);
        else
            cpy->children[i] = copy(gc, from->children[i], depth == TXC_NODE_DEPTH_MAX ? TXC_NODE_DEPTH_MAX : depth - 1);
    }
    switch (cpy->type) {
    case TXC_NAN:
        cpy->impl.reason = txc_strdup(from->impl.reason);
        if (cpy->impl.reason == NULL) {
            TXC_ERROR_ALLOC(strlen(from->impl.reason) + 1, "copy reason");
            if (cpy->children_amount > 0) {
                for (size_t i = 0; i < cpy->children_amount; i++) {
                    if (depth == 0)
                        txc_mem_gc_free(gc, cpy->children[i]);
                    else
                        txc_node_free(cpy->children[i], depth == TXC_NODE_DEPTH_MAX ? TXC_NODE_DEPTH_MAX : depth - 1);
                }
            }
            free(cpy);
            return (struct txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        break;
    case TXC_INT:
        cpy->impl.integer = txc_int_copy(from->impl.integer);
        if (cpy->impl.integer == NULL) {
            free(cpy);
            return (struct txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        break;
    case TXC_NEG: /* FALLTHROUGH */
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL: /* FALLTHROUGH */
    case TXC_FRAC:
        break;
    default:
        TXC_ERROR_INVALID_NODE_TYPE(cpy->type);
        break;
    }
    return cpy;
}

/*
const struct txc_node *txc_node_copy_read(const struct txc_node *const from)
{
    // TODO catch read only
    assert(txc_node_test_valid(from, true));
    return copy(from, false);
}

struct txc_node *txc_node_copy_write(const struct txc_node *const from)
{
    assert(txc_node_test_valid(from, true));
    return copy(from, true);
}
*/

void txc_node_free(const struct txc_node *const node, const TXC_NODE_DEPTH_TYPE depth)
{
    if (node == NULL)
        return;
    // TODO change back to true?
    assert(txc_node_test_valid(node, false));
    if (node->read_only)
        return;
    // TODO recursive
    for (size_t i = 0; i < node->children_amount && depth > 0; i++)
        txc_node_free(node->children[i], depth == TXC_NODE_DEPTH_MAX ? TXC_NODE_DEPTH_MAX : depth - 1);
    switch (node->type) {
    case TXC_NAN:
        free(node->impl.reason);
        break;
    case TXC_INT:
        txc_int_free(node->impl.integer);
        break;
    case TXC_NEG: /* FALLTHROUGH */
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL: /* FALLTHROUGH */
    case TXC_FRAC:
        break;
    default:
        TXC_ERROR_INVALID_NODE_TYPE(node->type);
        break;
    }
    free((struct txc_node *)node);
}

/* NODE */

const struct txc_node *txc_node_simplify(txc_mem_gc *gc, const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    struct txc_node *cpy = copy(gc, node, 0);
    for (size_t i = 0; i < node->children_amount; i++) {
        txc_mem_gc_free(gc, cpy->children[i]);
        cpy->children[i] = (struct txc_node *)txc_node_simplify(gc, node->children[i]);
        if (node->children[i]->type != TXC_NAN)
            continue;
        struct txc_node *tmp = txc_mem_gc_copy(gc, cpy->children[i]);
        txc_node_free(cpy, 0);
        return tmp;
    }
    switch (cpy->type) {
    case TXC_NEG:
        switch (cpy->children[0]->type) {
        case TXC_INT: {
            txc_int *const tmp = txc_int_copy(node->children[0]->impl.integer);
            txc_node_free(cpy, 0);
            if (tmp == NULL)
                return &TXC_NAN_ERROR_ALLOC;
            return txc_mem_gc_bake(gc, txc_int_to_node(gc, txc_int_neg(tmp)));
        }
        case TXC_NEG: {
            const struct txc_node *const tmp = txc_mem_gc_copy(gc, cpy->children[0]->children[0]);
            txc_node_free(cpy, 0);
            return tmp;
        }
        // TODO shift up/down?
        default:
            return txc_mem_gc_bake(gc, cpy);
        }
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL: {
        assert(cpy->read_only == false);
        // TODO handle empty sum/product
        cpy->children_amount = 0;
        for (size_t i = 0; i < node->children_amount; i++) {
            if (cpy->children[i]->type == cpy->type)
                cpy->children_amount += node->children[i]->children_amount;
            else
                cpy->children_amount++;
        }
        size_t int_i = 0;
        size_t other_i = 0;
        struct txc_node *int_nodes[cpy->children_amount];
        struct txc_node *other_nodes[cpy->children_amount];
        const struct txc_node *nan = NULL;
        // TODO useless since recursive?
        for (size_t i = 0; i < cpy->children_amount; i++) {
            if (cpy->children[i]->type == TXC_INT) {
                int_nodes[int_i] = cpy->children[i];
                int_i++;
                continue;
            }
            if (cpy->children[i]->type != cpy->type) {
                other_nodes[other_i] = cpy->children[i];
                other_i++;
                continue;
            }
            for (size_t j = 0; j < cpy->children[i]->children_amount; j++) {
                if (cpy->children[i]->type == TXC_INT) {
                    int_nodes[int_i] = copy(gc, cpy->children[i], 0);
                    if (int_nodes[int_i]->type == TXC_NAN && nan == NULL)
                        nan = txc_mem_gc_copy(gc, int_nodes[int_i]);
                    int_i++;
                } else {
                    other_nodes[other_i] = copy(gc, cpy->children[i], 0);
                    if (other_nodes[other_i]->type == TXC_NAN && nan == NULL)
                        nan = txc_mem_gc_copy(gc, other_nodes[other_i]);
                    other_i++;
                }
            }
            txc_mem_gc_free(gc, cpy->children[i]);
        }
        if (nan != NULL) {
            cpy->children_amount = 0;
            for (size_t i = 0; i < int_i; i++)
                txc_node_free(int_nodes[i], 0);
            for (size_t i = 0; i < other_i; i++)
                txc_node_free(other_nodes[i], 0);
            txc_node_free(cpy, 0);
            return nan;
        }
        cpy->children_amount = other_i + (int_i > 0 ? 1 : 0);
        struct txc_node *const tmp = realloc(cpy, sizeof *tmp + sizeof *tmp->children * cpy->children_amount);
        if (tmp == NULL) {
            cpy->children_amount = 0;
            for (size_t i = 0; i < int_i; i++)
                txc_node_free(int_nodes[i], 0);
            for (size_t i = 0; i < other_i; i++)
                txc_node_free(other_nodes[i], 0);
            TXC_ERROR_ALLOC(sizeof *tmp + sizeof *tmp->children * cpy->children_amount, "compact copy");
            txc_node_free(cpy, 0);
            return &TXC_NAN_ERROR_ALLOC;
        }
        cpy = tmp;
        for (size_t i = 0; i < other_i; i++)
            cpy->children[i] = other_nodes[i];
        if (int_i == 0)
            return txc_mem_gc_bake(gc, cpy);
        const txc_int *ints[int_i];
        for (size_t i = 0; i < int_i; i++)
            ints[i] = txc_node_to_int(int_nodes[i]);
        txc_int *integer;
        switch (cpy->type) {
        case TXC_ADD:
            integer = txc_int_add(ints, int_i);
            break;
        case TXC_MUL:
            integer = txc_int_mul(ints, int_i);
            break;
        default:
            for (size_t i = 0; i < int_i; i++)
                txc_node_free(int_nodes[i]);
            cpy->children_amount--;
            txc_node_free(cpy);
            TXC_ERROR_INVALID_NODE_TYPE(cpy->type);
            return &TXC_NAN_ERROR_INVALID_NODE_TYPE;
        }
        for (size_t i = 0; i < int_i; i++)
            txc_node_free(int_nodes[i]);
        const struct txc_node *const int_node = txc_int_to_node(gc, integer);
        if (int_node->type == TXC_NAN) {
            cpy->children_amount--;
            txc_node_free(cpy);
            return &TXC_NAN_ERROR_ALLOC;
        }
        if (cpy->children_amount == 1) {
            cpy->children_amount = 0;
            txc_node_free(cpy);
            return txc_mem_gc_bake(gc, int_node);
        }
        cpy->children[cpy->children_amount - 1] = (struct txc_node *)int_node;
        return txc_mem_gc_bake(gc, cpy);
    }
    case TXC_FRAC: {
        if (cpy->children[0]->type == TXC_INT && txc_int_is_zero(cpy->children[0]->impl.integer)) {
            txc_node_free(cpy);
            return &TXC_NAN_ZERO_DIVISION;
        }
        bool neg = false;
        if ((cpy->children[0]->type == TXC_INT && txc_int_is_neg(cpy->children[0]->impl.integer)) || cpy->children[0]->type == TXC_NEG) {
            neg = true;
            const struct txc_node *tmp;
            if (cpy->children[0]->type == TXC_NEG)
                tmp = copy(cpy->children[0]->children[0]);
            else
                tmp = txc_int_to_node(gc, txc_int_neg(txc_int_copy(cpy->children[0]->impl.integer)));
            txc_node_free(cpy->children[0]);
            cpy->children[0] = txc_mem_gc_bake(gc, tmp);
        }
        if (cpy->children_amount == 1) {
            const struct txc_node *const_copy;
            if (txc_int_is_pos_one(cpy->children[0]->impl.integer)) {
                const_copy = txc_mem_gc_copy(gc, cpy->children[0]);
                txc_node_free(cpy);
            } else {
                const_copy = txc_mem_gc_bake(gc, cpy);
            }
            if (!neg)
                return txc_mem_gc_bake(gc, cpy);
            const struct txc_node *const tmp1 = txc_node_create_un_op(gc, TXC_NEG, const_copy);
            txc_node_free(const_copy);
            struct txc_node *tmp2 = txc_node_simplify(gc, tmp1);
            txc_node_free(tmp1);
            return tmp2;
        }
        if ((cpy->children[1]->type == TXC_INT && txc_int_is_neg(cpy->children[1]->impl.integer)) || cpy->children[1]->type == TXC_NEG) {
            neg = !neg;
            const struct txc_node *tmp;
            if (cpy->children[1]->type == TXC_NEG)
                tmp = copy(cpy->children[1]->children[0]);
            else
                tmp = txc_int_to_node(gc, txc_int_neg(txc_int_copy(cpy->children[1]->impl.integer)));
            txc_node_free(cpy->children[1]);
            cpy->children[1] = txc_mem_gc_bake(gc, tmp);
        }
        if (cpy->children[0]->type == TXC_INT && txc_int_is_pos_one(cpy->children[0]->impl.integer)) {
            const struct txc_node *tmp = txc_mem_gc_copy(gc, cpy->children[1]);
            txc_node_free(cpy);
            if (neg) {
                const struct txc_node *const tmp_2 = txc_node_create_un_op(gc, TXC_NEG, txc_mem_gc_copy(gc, tmp));
                txc_node_free(tmp);
                tmp = tmp_2;
            }
            return txc_node_simplify(gc, tmp);
        }
        // TODO switch if frac
        if (cpy->children[0]->type == TXC_NAN || cpy->children[1]->type == TXC_NAN) {
            const struct txc_node *const tmp = txc_mem_gc_copy(gc, cpy->children[cpy->children[0]->type == TXC_NAN ? 0 : 1]);
            txc_node_free(cpy);
            return tmp;
        }
        const txc_int *num = NULL;
        const txc_int *den = NULL;
        if (cpy->children[0]->type == TXC_INT)
            den = txc_int_copy(cpy->children[0]->impl.integer);
        else if (cpy->children[0]->type == TXC_MUL && cpy->children[0]->children[cpy->children[0]->children_amount - 1]->type == TXC_INT)
            den = txc_int_copy(cpy->children[0]->children[cpy->children[0]->children_amount - 1]->impl.integer);
        if (cpy->children[1]->type == TXC_INT)
            num = txc_int_copy(cpy->children[1]->impl.integer);
        else if (cpy->children[1]->type == TXC_MUL && cpy->children[1]->children[cpy->children[1]->children_amount - 1]->type == TXC_INT)
            num = txc_int_copy(cpy->children[1]->children[cpy->children[1]->children_amount - 1]->impl.integer);
        const txc_int *const gcd = txc_int_gcd(num, den);
        const struct txc_node *const num_node = txc_int_to_node(gc, txc_int_div(num, gcd));
        if (num_node->type == TXC_NAN) {
            txc_int_free(num);
            txc_int_free(den);
            txc_int_free(gcd);
            txc_node_free(cpy);
            return &TXC_NAN_ERROR_ALLOC;
        }
        txc_int_free(num);
        const struct txc_node *const den_node = txc_int_to_node(gc, txc_int_div(den, gcd));
        if (den_node->type == TXC_NAN) {
            txc_int_free(den);
            txc_int_free(gcd);
            txc_node_free(num_node);
            txc_node_free(cpy);
            return &TXC_NAN_ERROR_ALLOC;
        }
        txc_int_free(den);
        txc_int_free(gcd);
        if (cpy->children[0]->type == TXC_INT) {
            txc_node_free(cpy->children[0]);
            cpy->children[0] = den_node;
        } else {
            txc_node_free(cpy->children[0]->children[cpy->children[0]->children_amount - 1]);
            cpy->children[0]->children[cpy->children[0]->children_amount - 1] = den_node;
            cpy->children[0] = txc_node_simplify(gc, cpy->children[0]);
            if (cpy->children[0]->type == TXC_NAN) {
                const struct txc_node *const tmp = txc_mem_gc_copy(gc, cpy->children[0]);
                txc_node_free(cpy);
                return tmp;
            }
        }
        if (cpy->children[1]->type == TXC_INT) {
            txc_node_free(cpy->children[1]);
            cpy->children[1] = num_node;
        } else {
            txc_node_free(cpy->children[1]->children[cpy->children[1]->children_amount - 1]);
            cpy->children[1]->children[cpy->children[1]->children_amount - 1] = num_node;
            cpy->children[1] = txc_node_simplify(gc, cpy->children[1]);
            if (cpy->children[1]->type == TXC_NAN) {
                const struct txc_node *const tmp = txc_mem_gc_copy(gc, cpy->children[1]);
                txc_node_free(cpy);
                return tmp;
            }
        }
        if (cpy->children[0]->type == TXC_INT && txc_int_is_pos_one(cpy->children[0]->impl.integer)) {
            const struct txc_node *tmp_1 = txc_mem_gc_copy(gc, cpy->children[1]);
            txc_node_free(cpy);
            if (neg) {
                const struct txc_node *const tmp_2 = txc_node_create_un_op(gc, TXC_NEG, txc_mem_gc_copy(gc, tmp_1));
                txc_node_free(tmp_1);
                tmp_1 = txc_node_simplify(gc, tmp_2);
            }
            return tmp_1;
        }
        if (cpy->children[1]->type == TXC_INT && txc_int_is_pos_one(cpy->children[1]->impl.integer)) {
            txc_node_free(cpy->children[1]);
            cpy->children_amount = 1;
            struct txc_node *tmp = realloc(cpy, sizeof *cpy + sizeof *cpy->children * 1);
            if (tmp == NULL) {
                TXC_ERROR_ALLOC(sizeof *cpy + sizeof *cpy->children * 1, "unit fraction");
                txc_node_free(cpy);
                return &TXC_NAN_ERROR_ALLOC;
            }
            cpy = tmp;
        }
        if (neg) {
            const struct txc_node *const tmp = txc_node_create_un_op(gc, TXC_NEG, cpy);
            txc_node_free(cpy);
            return tmp;
        }
        return txc_mem_gc_bake(gc, cpy);
    }
    case TXC_NAN: /* FALLTHROUGH */
    case TXC_INT: /* FALLTHROUGH */
    default:
        return txc_mem_gc_bake(gc, cpy);
    }
}

/* PRINT */

static char *concat_children_in_paren(const struct txc_node *const *const children, const size_t amount, const bool reverse_children, const char *const pre, const char *const op, const char *const post)
{
    for (size_t i = 0; i < amount; i++)
        assert(txc_node_test_valid(children[i], true));
    size_t size = strlen(pre) + strlen(op) * (amount - 1) + strlen(post);
    char *child_strs[amount];
    for (size_t i = 0; i < amount; i++) {
        child_strs[i] = txc_node_to_str(children[reverse_children ? amount - i - 1 : i]);
        if (child_strs[i] == NULL) {
            for (size_t j = 0; j < i; j++)
                free(child_strs[j]);
            return NULL;
        }
        size += strlen(child_strs[i]);
    }
    char *const buf = malloc(sizeof *buf * (size + 1));
    if (buf == NULL) {
        TXC_ERROR_ALLOC(sizeof *buf * (size + 1), "concatinated string");
        for (size_t i = 0; i < amount; i++)
            free(child_strs[i]);
        return txc_node_to_str(&TXC_NAN_ERROR_ALLOC);
    }
    char *cur = txc_stpcpy(buf, pre);
    for (size_t i = 0; i < amount; i++) {
        if (i > 0)
            cur = txc_stpcpy(cur, op);
        cur = txc_stpcpy(cur, child_strs[i]);
    }
    txc_stpcpy(cur, post);
    for (size_t i = 0; i < amount; i++)
        free(child_strs[i]);
    return buf;
}

char *txc_node_to_str(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    char *str = NULL;
    switch (node->type) {
    case TXC_NAN: {
        size_t size = sizeof str * (strlen(TXC_NAN_REASON) + strlen(node->impl.reason) + 1);
        str = malloc(size);
        if (str == NULL)
            TXC_ERROR_ALLOC(size, "nan string");
        else
            snprintf(str, size, TXC_NAN_REASON, node->impl.reason);
        return str;
    }
    case TXC_INT:
        str = txc_int_to_str(node->impl.integer);
        if (str == NULL)
            return txc_node_to_str(&TXC_NAN_ERROR_ALLOC);
        return str;
    case TXC_NEG:
        return concat_children_in_paren((const struct txc_node *const *)node->children, node->children_amount, false, "(-", "", ")");
    case TXC_ADD:
        return concat_children_in_paren((const struct txc_node *const *)node->children, node->children_amount, false, "(", " + ", ")");
    case TXC_MUL:
        return concat_children_in_paren((const struct txc_node *const *)node->children, node->children_amount, false, "(", " \\cdot ", ")");
    case TXC_FRAC:
        if (node->children_amount == 1)
            return concat_children_in_paren((const struct txc_node *const *)node->children, node->children_amount, true, "\\frac{1}{", "", "}");
        else
            return concat_children_in_paren((const struct txc_node *const *)node->children, node->children_amount, true, "\\frac{", "}{", "}");
    default:
        TXC_ERROR_INVALID_NODE_TYPE(node->type);
        return txc_node_to_str(&TXC_NAN_ERROR_INVALID_NODE_TYPE);
    }
}

void txc_node_print(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    char *str = txc_node_to_str(node);
    if (str == NULL) {
        printf(TXC_PRINT_FORMAT, TXC_PRINT_ERROR);
    } else {
        printf(TXC_PRINT_FORMAT, str);
        free(str);
    }
}

void txc_node_print_if_debug(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
#if DEBUG
    txc_node_print(node);
#endif /* DEBUG */
}

void txc_node_simplify_and_print(txc_mem_gc *const gc, const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    txc_node_print_if_debug(node);
    const struct txc_node *const simple_node = txc_node_simplify(gc, node);
    txc_node_print(simple_node);
    txc_node_free(simple_node);
}
