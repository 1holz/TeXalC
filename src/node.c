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
#include "util.h"

#define TXC_NAN_REASON "\\text{NAN(%s)}"
#define TXC_NAN_REASON_ERROR_NYI "Not yet implemented. Please see stderr for more information."
#define TXC_NAN_REASON_ERROR_ALLOC "Could not allocate enough memory. Please see stderr for more information."
#define TXC_NAN_REASON_ERROR_INVALID_NODE_TYPE "Node type is invalid. Please see stderr for more information."
#define TXC_NAN_REASON_UNSPECIFIED "unspecified"
#define TXC_NAN_REASON_ZERO_DIVISION "Divided by 0."

#define TXC_PRINT_FORMAT "= %s \\\\\n"
#define TXC_PRINT_FORMAT_NEWLINE "= %s \\\\\n\n"
#define TXC_PRINT_ERROR "\\text{PRINT(Unable to print. Please see stderr for more information.)}"

/* DEFINITIONS */

struct txc_node {
    struct txc_node **children;
    union impl impl;
    size_t children_amount;
    enum txc_node_type type;
    bool read_only;
};

/* CONSTANTS */

const struct txc_node TXC_NAN_ERROR_ALLOC = { .children = NULL,
                                              .impl.reason = TXC_NAN_REASON_ERROR_ALLOC,
                                              .children_amount = 0,
                                              .type = TXC_NAN,
                                              .read_only = true };
const struct txc_node TXC_NAN_ERROR_INVALID_NODE_TYPE = { .children = NULL,
                                                          .impl.reason = TXC_NAN_REASON_ERROR_INVALID_NODE_TYPE,
                                                          .children_amount = 0,
                                                          .type = TXC_NAN,
                                                          .read_only = true };
const struct txc_node TXC_NAN_ERROR_NYI = { .children = NULL,
                                            .impl.reason = TXC_NAN_REASON_ERROR_NYI,
                                            .children_amount = 0,
                                            .type = TXC_NAN,
                                            .read_only = true };
const struct txc_node TXC_NAN_UNSPECIFIED = { .children = NULL,
                                              .impl.reason = TXC_NAN_REASON_UNSPECIFIED,
                                              .children_amount = 0,
                                              .type = TXC_NAN,
                                              .read_only = true };
const struct txc_node TXC_NAN_ZERO_DIVISION = { .children = NULL,
                                                .impl.reason = TXC_NAN_REASON_ZERO_DIVISION,
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
    if (node->children_amount > 0 && node->children == NULL) {
        TXC_ERROR_NULL("node children");
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

/* MEMORY */

static const struct txc_node *bake(struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    return node;
}

const txc_int *txc_node_to_int(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    return node->type == TXC_INT ? node->impl.integer : NULL;
}

const struct txc_node *txc_node_create(const struct txc_node *const *const children, const union impl impl, const size_t children_amount, const enum txc_node_type type)
{
    for (size_t i = 0; i < children_amount; i++)
        assert(txc_node_test_valid(children[i], true));
    struct txc_node *const node = malloc(sizeof *node);
    if (node == NULL) {
        TXC_ERROR_ALLOC(sizeof *node, "node");
        return &TXC_NAN_ERROR_ALLOC;
    }
    node->children = (struct txc_node **)children;
    node->impl = impl;
    node->children_amount = children_amount;
    node->type = type;
    node->read_only = false;
    return bake(node);
}

const struct txc_node *txc_node_create_nan(const char *const reason)
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
    nan->type = TXC_NAN;
    nan->read_only = false;
    return bake(nan);
}

static const struct txc_node *init_op(const enum txc_node_type type, const struct txc_node *const *const children, const size_t children_amount)
{
    for (size_t i = 0; i < children_amount; i++)
        assert(txc_node_test_valid(children[i], true));
    struct txc_node *const node = malloc(sizeof *node);
    if (node == NULL) {
        TXC_ERROR_ALLOC(sizeof *node, "operation node");
        return &TXC_NAN_ERROR_ALLOC;
    }
    node->children_amount = children_amount;
    node->children = (struct txc_node **)children;
    node->type = type;
    node->read_only = false;
    return bake(node);
}

const struct txc_node *txc_node_create_un_op(const enum txc_node_type type, const struct txc_node *const operand)
{
    if (type != TXC_NEG && type != TXC_FRAC) {
        TXC_ERROR_INVALID_NODE_TYPE(type);
        return &TXC_NAN_ERROR_INVALID_NODE_TYPE;
    }
    assert(txc_node_test_valid(operand, true));
    const size_t arity = 1;
    const struct txc_node **const children = malloc(sizeof *children * arity);
    if (children == NULL) {
        TXC_ERROR_ALLOC(sizeof *children * arity, "unary operation node children");
        return &TXC_NAN_ERROR_ALLOC;
    }
    children[0] = operand;
    return init_op(type, children, arity);
}

const struct txc_node *txc_node_create_bin_op(const enum txc_node_type type, const struct txc_node *const operand_1, const struct txc_node *const operand_2)
{
    if (type != TXC_ADD && type != TXC_MUL && type != TXC_FRAC) {
        TXC_ERROR_INVALID_NODE_TYPE(type);
        return &TXC_NAN_ERROR_INVALID_NODE_TYPE;
    }
    assert(txc_node_test_valid(operand_1, true));
    assert(txc_node_test_valid(operand_2, true));
    const size_t arity = 2;
    const struct txc_node **const children = malloc(sizeof *children * arity);
    if (children == NULL) {
        TXC_ERROR_ALLOC(sizeof *children * arity, "binary operation node children");
        return &TXC_NAN_ERROR_ALLOC;
    }
    children[0] = operand_1;
    children[1] = operand_2;
    return init_op(type, children, arity);
}

static struct txc_node *copy(const struct txc_node *const from, const bool write)
{
    assert(txc_node_test_valid(from, true));
    if (from->read_only)
        return (struct txc_node *)from;
    struct txc_node *copy = malloc(sizeof *copy);
    if (copy == NULL) {
        TXC_ERROR_ALLOC(sizeof *copy, "copy");
        return (struct txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    copy->children_amount = from->children_amount;
    copy->type = from->type;
    copy->read_only = false;
    if (copy->children_amount > 0) {
        copy->children = malloc(sizeof *copy->children * copy->children_amount);
        if (copy->children == NULL) {
            TXC_ERROR_ALLOC(sizeof *copy->children * copy->children_amount, "copy children");
            free(copy);
            return (struct txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        for (size_t i = 0; i < copy->children_amount; i++)
            copy->children[i] = write ? txc_node_copy_write(from->children[i]) : (struct txc_node *)txc_node_copy_read(from->children[i]);
    }
    switch (copy->type) {
    case TXC_NAN:
        copy->impl.reason = txc_strdup(from->impl.reason);
        if (copy->impl.reason == NULL) {
            TXC_ERROR_ALLOC(strlen(from->impl.reason) + 1, "copy reason");
            if (copy->children_amount > 0) {
                for (size_t i = 0; i < copy->children_amount; i++)
                    txc_node_free(copy->children[i]);
                free(copy->children);
            }
            free(copy);
            return (struct txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        break;
    case TXC_INT:
        copy->impl.integer = write ? txc_int_copy(from->impl.integer) : (txc_int *)txc_int_copy(from->impl.integer);
        if (copy->impl.integer == NULL) {
            free(copy);
            return (struct txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        break;
    case TXC_NEG: /* FALLTHROUGH */
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL: /* FALLTHROUGH */
    case TXC_FRAC:
        break;
    default:
        TXC_ERROR_INVALID_NODE_TYPE(copy->type);
        break;
    }
    return copy;
}

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

void txc_node_free(const struct txc_node *const node)
{
    if (node == NULL)
        return;
    // TODO change back to true?
    assert(txc_node_test_valid(node, false));
    if (node->read_only)
        return;
    for (size_t i = 0; i < node->children_amount; i++)
        txc_node_free(node->children[i]);
    if (node->children_amount > 0)
        free(node->children);
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

const struct txc_node *txc_node_simplify(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    struct txc_node *copy = txc_node_copy_write(node);
    for (size_t i = 0; i < node->children_amount; i++) {
        txc_node_free(copy->children[i]);
        copy->children[i] = (struct txc_node *)txc_node_simplify(node->children[i]);
        if (node->children[i]->type != TXC_NAN)
            continue;
        const struct txc_node *const tmp = txc_node_copy_read(copy->children[i]);
        txc_node_free(copy);
        return tmp;
    }
    switch (copy->type) {
    case TXC_NEG:
        switch (copy->children[0]->type) {
        case TXC_INT: {
            txc_int *const tmp = txc_int_copy(node->children[0]->impl.integer);
            txc_node_free(copy);
            if (tmp == NULL)
                return &TXC_NAN_ERROR_ALLOC;
            return txc_int_to_node(txc_int_neg(tmp));
        }
        case TXC_NEG: {
            const struct txc_node *const tmp = txc_node_copy_read(copy->children[0]->children[0]);
            txc_node_free(copy);
            return tmp;
        }
        // TODO shift up/down?
        default:
            return bake(copy);
        }
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL: {
        assert(copy->read_only == false);
        // TODO handle empty sum/product
        copy->children_amount = 0;
        for (size_t i = 0; i < node->children_amount; i++) {
            if (copy->children[i]->type == copy->type)
                copy->children_amount += node->children[i]->children_amount;
            else
                copy->children_amount++;
        }
        struct txc_node **const tmp1 = malloc(sizeof *copy->children * copy->children_amount);
        if (tmp1 == NULL) {
            TXC_ERROR_ALLOC(sizeof *copy->children * copy->children_amount, "compact copy");
            txc_node_free(copy);
            return &TXC_NAN_ERROR_ALLOC;
        }
        size_t ii = 0;
        for (size_t j = 0; ii < copy->children_amount; j++) {
            if (copy->children[j]->type != copy->type) {
                tmp1[ii] = txc_node_copy_write(copy->children[j]);
                ii++;
            } else {
                for (size_t k = 0; k < copy->children[j]->children_amount && ii < copy->children_amount; k++) {
                    tmp1[ii] = txc_node_copy_write(copy->children[j]->children[k]);
                    ii++;
                }
            }
            txc_node_free(copy->children[j]);
        }
        free(copy->children);
        copy->children = tmp1;
        // TODO compacting finished
        size_t int_i = 0;
        size_t other_i = 0;
        for (size_t i = 0; i < copy->children_amount; i++) {
            if (copy->children[i]->type == TXC_INT)
                int_i++;
        }
        if (int_i <= 0)
            return bake(copy);
        const txc_int *ints[int_i];
        struct txc_node *int_nodes[int_i];
        int_i = 0;
        for (size_t i = 0; i < copy->children_amount; i++) {
            if (copy->children[i]->type == TXC_INT) {
                ints[int_i] = txc_node_to_int(copy->children[i]);
                int_nodes[int_i] = copy->children[i];
                int_i++;
            } else {
                copy->children[other_i] = copy->children[i];
                other_i++;
            }
        }
        copy->children_amount = other_i + 1;
        struct txc_node **tmp2 = realloc(copy->children, sizeof *tmp2 * copy->children_amount);
        if (tmp2 == NULL) {
            txc_node_free(copy);
            TXC_ERROR_ALLOC(sizeof *tmp2 * copy->children_amount, "others only children");
            return &TXC_NAN_ERROR_ALLOC;
        }
        copy->children = tmp2;
        txc_int *integer;
        switch (copy->type) {
        case TXC_ADD:
            integer = txc_int_add(ints, int_i);
            break;
        case TXC_MUL:
            integer = txc_int_mul(ints, int_i);
            break;
        default:
            for (size_t i = 0; i < int_i; i++)
                txc_node_free(int_nodes[i]);
            copy->children[copy->children_amount - 1] = NULL;
            txc_node_free(copy);
            TXC_ERROR_INVALID_NODE_TYPE(copy->type);
            return &TXC_NAN_ERROR_INVALID_NODE_TYPE;
        }
        for (size_t i = 0; i < int_i; i++)
            txc_node_free(int_nodes[i]);
        if (integer == NULL) {
            copy->children[copy->children_amount - 1] = NULL;
            txc_node_free(copy);
            return &TXC_NAN_ERROR_ALLOC;
        }
        const struct txc_node *const int_node = txc_int_to_node(integer);
        if (copy->children_amount <= 1) {
            copy->children[copy->children_amount - 1] = NULL;
            txc_node_free(copy);
            return int_node;
        }
        copy->children[copy->children_amount - 1] = (struct txc_node *)int_node;
        return bake(copy);
    }
    case TXC_FRAC: {
        if (copy->children[0]->type == TXC_INT && txc_int_is_zero(copy->children[0]->impl.integer)) {
            txc_node_free(copy);
            return &TXC_NAN_ZERO_DIVISION;
        }
        bool neg = false;
        if ((copy->children[0]->type == TXC_INT && txc_int_is_neg(copy->children[0]->impl.integer)) || copy->children[0]->type == TXC_NEG) {
            neg = true;
            const struct txc_node *tmp;
            if (copy->children[0]->type == TXC_NEG)
                tmp = txc_node_copy_read(copy->children[0]->children[0]);
            else
                tmp = txc_int_to_node(txc_int_neg(txc_int_copy(copy->children[0]->impl.integer)));
            txc_node_free(copy->children[0]);
            copy->children[0] = txc_node_copy_write(tmp);
            txc_node_free(tmp);
        }
        if (copy->children_amount == 1) {
            const struct txc_node *const_copy;
            if (txc_int_is_pos_one(copy->children[0]->impl.integer)) {
                const_copy = txc_node_copy_read(copy->children[0]);
                txc_node_free(copy);
            } else {
                const_copy = bake(copy);
            }
            if (!neg)
                return bake(copy);
            const struct txc_node *const tmp1 = txc_node_create_un_op(TXC_NEG, const_copy);
            txc_node_free(const_copy);
            const struct txc_node *const tmp2 = txc_node_simplify(tmp1);
            txc_node_free(tmp1);
            return tmp2;
        }
        if ((copy->children[1]->type == TXC_INT && txc_int_is_neg(copy->children[1]->impl.integer)) || copy->children[1]->type == TXC_NEG) {
            neg = !neg;
            const struct txc_node *tmp;
            if (copy->children[1]->type == TXC_NEG)
                tmp = txc_node_copy_read(copy->children[1]->children[0]);
            else
                tmp = txc_int_to_node(txc_int_neg(txc_int_copy(copy->children[1]->impl.integer)));
            txc_node_free(copy->children[1]);
            copy->children[1] = txc_node_copy_write(tmp);
            txc_node_free(tmp);
        }
        if (copy->children[0]->type == TXC_INT && txc_int_is_pos_one(copy->children[0]->impl.integer)) {
            const struct txc_node *tmp = txc_node_copy_read(copy->children[1]);
            txc_node_free(copy);
            if (neg) {
                const struct txc_node *const tmptmp = txc_node_create_un_op(TXC_NEG, txc_node_copy_read(tmp));
                txc_node_free(tmp);
                tmp = tmptmp;
            }
            return txc_node_simplify(tmp);
        }
        // TODO switch if frac
        if (copy->children[0]->type == TXC_NAN || copy->children[1]->type == TXC_NAN) {
            const struct txc_node *const tmp = txc_node_copy_read(copy->children[copy->children[0]->type == TXC_NAN ? 0 : 1]);
            txc_node_free(copy);
            return tmp;
        }
        const txc_int *num = NULL;
        const txc_int *den = NULL;
        // TODO free here already?
        if (copy->children[0]->type == TXC_INT)
            den = txc_int_copy(copy->children[0]->impl.integer);
        else if (copy->children[0]->type == TXC_MUL && copy->children[0]->children[copy->children[0]->children_amount - 1]->type == TXC_INT)
            den = txc_int_copy(copy->children[0]->children[copy->children[0]->children_amount - 1]->impl.integer);
        if (copy->children[1]->type == TXC_INT)
            num = txc_int_copy(copy->children[1]->impl.integer);
        else if (copy->children[1]->type == TXC_MUL && copy->children[1]->children[copy->children[1]->children_amount - 1]->type == TXC_INT)
            num = txc_int_copy(copy->children[1]->children[copy->children[1]->children_amount - 1]->impl.integer);
        if (num == NULL || den == NULL) {
            txc_int_free(num);
            txc_int_free(den);
            return bake(copy);
        }
        const txc_int *const gcd = txc_int_gcd(num, den);
        if (gcd == NULL) {
            txc_node_free(copy);
            return &TXC_NAN_ERROR_ALLOC;
        }
        const struct txc_node *const num_node = txc_int_to_node((txc_int *)txc_int_div(num, gcd));
        if (num_node->type == TXC_NAN) {
            txc_int_free(num);
            txc_int_free(den);
            txc_node_free(copy);
            return num_node;
        }
        txc_int_free(num);
        const struct txc_node *const den_node = txc_int_to_node((txc_int *)txc_int_div(den, gcd));
        if (den_node->type == TXC_NAN) {
            txc_int_free(den);
            txc_node_free(num_node);
            txc_node_free(copy);
            return den_node;
        }
        txc_int_free(den);
        txc_int_free(gcd);
        if (copy->children[0]->type == TXC_INT) {
            txc_node_free(copy->children[0]);
            copy->children[0] = den_node;
        } else {
            txc_node_free(copy->children[0]->children[copy->children[0]->children_amount - 1]);
            copy->children[0]->children[copy->children[0]->children_amount - 1] = den_node;
            copy->children[0] = txc_node_simplify(copy->children[0]);
            if (copy->children[0]->type == TXC_NAN) {
                const struct txc_node *const tmp = txc_node_copy_read(copy->children[0]);
                txc_node_free(copy);
                return tmp;
            }
        }
        if (copy->children[1]->type == TXC_INT) {
            txc_node_free(copy->children[1]);
            copy->children[1] = num_node;
        } else {
            txc_node_free(copy->children[1]->children[copy->children[1]->children_amount - 1]);
            copy->children[1]->children[copy->children[1]->children_amount - 1] = num_node;
            copy->children[1] = txc_node_simplify(copy->children[1]);
            if (copy->children[1]->type == TXC_NAN) {
                const struct txc_node *const tmp = txc_node_copy_read(copy->children[1]);
                txc_node_free(copy);
                return tmp;
            }
        }
        if (copy->children[0]->type == TXC_INT && txc_int_is_pos_one(copy->children[0]->impl.integer)) {
            const struct txc_node *tmp = txc_node_copy_read(copy->children[1]);
            txc_node_free(copy);
            if (neg) {
                const struct txc_node *const tmptmp = txc_node_create_un_op(TXC_NEG, txc_node_copy_read(tmp));
                txc_node_free(tmp);
                tmp = txc_node_simplify(tmptmp);
            }
            return tmp;
        }
        if (copy->children[1]->type == TXC_INT && txc_int_is_pos_one(copy->children[1]->impl.integer)) {
            struct txc_node **tmp = realloc(copy->children, sizeof *copy->children * 1);
            if (tmp == NULL) {
                TXC_ERROR_ALLOC(sizeof *copy->children * 1, "unit fraction");
                txc_node_free(copy);
                return &TXC_NAN_ERROR_ALLOC;
            }
            txc_node_free(copy->children[1]);
            copy->children = tmp;
            copy->children_amount = 1;
        }
        if (neg) {
            const struct txc_node *const tmp = txc_node_create_un_op(TXC_NEG, copy);
            txc_node_free(copy);
            return tmp;
        }
        return bake(copy);
    }
    case TXC_NAN: /* FALLTHROUGH */
    case TXC_INT: /* FALLTHROUGH */
    default:
        return bake(copy);
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

void txc_node_simplify_and_print(const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    txc_node_print_if_debug(node);
    const struct txc_node *const simple_node = txc_node_simplify(node);
    txc_node_print(simple_node);
    txc_node_free(simple_node);
}
