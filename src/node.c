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
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "node.h"
#include "integers.h"
#include "util.h"

#define TXC_ERROR_ALLOC "Could not allocate %zu bytes of memory for %s in %s line %d.\n"
#define TXC_ERROR_INVALID_NODE_TYPE "Node type %du is invalid in %s line %d.\n"
#define TXC_ERROR_NYI "Not yet implemented in %s line %d.\n"

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

struct txc_node
{
    struct txc_node **children;
    union impl impl;
    size_t children_amount;
    enum txc_node_type type;
    bool read_only;
};

/* CONSTANTS */

const txc_node TXC_NAN_ERROR_ALLOC = {.children = NULL,
                                      .impl.reason = TXC_NAN_REASON_ERROR_ALLOC,
                                      .children_amount = 0,
                                      .type = TXC_NAN,
                                      .read_only = true};
const txc_node TXC_NAN_ERROR_INVALID_NODE_TYPE = {.children = NULL,
                                                  .impl.reason = TXC_NAN_REASON_ERROR_INVALID_NODE_TYPE,
                                                  .children_amount = 0,
                                                  .type = TXC_NAN,
                                                  .read_only = true};
const txc_node TXC_NAN_ERROR_NYI = {.children = NULL,
                                    .impl.reason = TXC_NAN_REASON_ERROR_NYI,
                                    .children_amount = 0,
                                    .type = TXC_NAN,
                                    .read_only = true};
const txc_node TXC_NAN_UNSPECIFIED = {.children = NULL,
                                      .impl.reason = TXC_NAN_REASON_UNSPECIFIED,
                                      .children_amount = 0,
                                      .type = TXC_NAN,
                                      .read_only = true};
const txc_node TXC_NAN_ZERO_DIVISION = {.children = NULL,
                                        .impl.reason = TXC_NAN_REASON_ZERO_DIVISION,
                                        .children_amount = 0,
                                        .type = TXC_NAN,
                                        .read_only = true};

/* ASSERTS */

static void node_assert_valid(const struct txc_node *const node)
{
    assert(node != NULL);
    switch (node->type)
    {
    case TXC_INT:
        assert(node->children_amount == 0);
        // TODO assert int?
        break;
    case TXC_NEG:
        assert(node->children_amount == 1);
        break;
    case TXC_FRAC:
        assert(node->children_amount >= 1 && node->children_amount <= 2);
        break;
    case TXC_NAN: /* FALLTHROUGH */
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL:
        break;
    default:
        fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, node->type, __FILE__, __LINE__);
        break;
    }
    if (node->children_amount <= 0)
        return;
    assert(node->children != NULL);
    for (size_t i = 0; i < node->children_amount; i++)
        node_assert_valid(node->children[i]);
}

/* CONVERT */

struct txc_int *txc_node_to_int(txc_node *const node)
{
    node_assert_valid(node);
    assert(node->type == TXC_INT);
    return node->impl.integer;
}

/* MEMORY */

static struct txc_node *bake(struct txc_node *const node)
{
    node_assert_valid(node);
    return node;
}

txc_node *txc_node_create(txc_node **children, union impl impl, size_t children_amount, enum txc_node_type type)
{
    txc_node *node = malloc(sizeof *node);
    if (node == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *node, "node", __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    node->children = children;
    node->impl = impl;
    node->children_amount = children_amount;
    node->type = type;
    node->read_only = false;
    return bake(node);
}

txc_node *txc_node_create_nan(const char *const reason)
{
    if (reason == NULL)
        return (txc_node *)&TXC_NAN_UNSPECIFIED;
    txc_node *nan = malloc(sizeof *nan);
    if (nan == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *nan, "nan", __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    nan->impl.reason = txc_strdup(reason);
    if (nan->impl.reason == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *nan, "nan reason", __FILE__, __LINE__);
        free(nan);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    nan->type = TXC_NAN;
    nan->read_only = false;
    return bake(nan);
}

static txc_node *init_op(const enum txc_node_type type, struct txc_node **children, size_t children_amount)
{
    txc_node *node = malloc(sizeof *node);
    if (node == NULL)
    {
        for (size_t i = 0; i < children_amount; i++)
            txc_node_free(children[i]);
        free(children);
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *node, "operation node", __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    node->children_amount = children_amount;
    node->children = children;
    node->type = type;
    node->read_only = false;
    return bake(node);
}

txc_node *txc_node_create_un_op(const enum txc_node_type type, txc_node *const operand)
{
    if (type != TXC_NEG && type != TXC_FRAC)
    {
        fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, type, __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_INVALID_NODE_TYPE;
    }
    size_t arity = 1;
    struct txc_node **children = malloc(sizeof *children * arity);
    if (children == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof children * arity, "unary operation node children", __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    children[0] = operand;
    return init_op(type, children, arity);
}

txc_node *txc_node_create_bin_op(const enum txc_node_type type, txc_node *const operand_1, txc_node *const operand_2)
{
    if (type != TXC_ADD && type != TXC_MUL && type != TXC_FRAC)
    {

        fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, type, __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_INVALID_NODE_TYPE;
    }
    size_t arity = 2;
    struct txc_node **children = malloc(sizeof *children * arity);
    if (children == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof children * arity, "binary operation node children", __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    children[0] = operand_1;
    children[1] = operand_2;
    return init_op(type, children, arity);
}

static struct txc_node *node_copy(const struct txc_node *const from, bool write)
{
    node_assert_valid(from);
    if (from->read_only)
        return (struct txc_node *)from;
    txc_node *copy = malloc(sizeof *copy);
    if (copy == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *copy, "copy", __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    copy->children_amount = from->children_amount;
    copy->type = from->type;
    copy->read_only = false;
    if (copy->children_amount > 0)
    {
        copy->children = malloc(sizeof *copy->children * copy->children_amount);
        if (copy->children == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, sizeof *copy, "copy children", __FILE__, __LINE__);
            free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        for (size_t i = 0; i < copy->children_amount; i++)
            copy->children[i] = write ? txc_node_copy_write(from->children[i]) : txc_node_copy_read(from->children[i]);
    }
    switch (copy->type)
    {
    case TXC_NAN:
        copy->impl.reason = txc_strdup(from->impl.reason);
        if (copy->impl.reason == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, sizeof *copy, "copy reason", __FILE__, __LINE__);
            if (copy->children_amount > 0)
            {
                for (size_t i = 0; i < copy->children_amount; i++)
                    txc_node_free(copy->children[i]);
                free(copy->children);
            }
            free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        break;
    case TXC_INT:
        copy->impl.integer = write ? txc_int_copy_write(from->impl.integer) : txc_int_copy_read(from->impl.integer);
        if (copy->impl.integer == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, sizeof *copy, "copy integer", __FILE__, __LINE__);
            free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        break;
    case TXC_NEG: /* FALLTHROUGH */
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL: /* FALLTHROUGH */
    case TXC_FRAC:
        break;
    default:
        fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, copy->type, __FILE__, __LINE__);
        break;
    }
    return copy;
}

txc_node *txc_node_copy_read(const txc_node *const from)
{
    node_assert_valid(from);
    return node_copy(from, false);
}

txc_node *txc_node_copy_write(const txc_node *const from)
{
    node_assert_valid(from);
    return node_copy(from, true);
}

void txc_node_free(txc_node *const node)
{
    node_assert_valid(node);
    if (node->read_only)
        return;
    for (size_t i = 0; i < node->children_amount; i++)
        txc_node_free(node->children[i]);
    if (node->children_amount > 0)
        free(node->children);
    switch (node->type)
    {
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
        fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, node->type, __FILE__, __LINE__);
        break;
    }
    free(node);
}

/* NODE */

txc_node *txc_node_simplify(txc_node *const node)
{
    node_assert_valid(node);
    for (size_t i = 0; i < node->children_amount; i++)
    {
        node->children[i] = txc_node_simplify(node->children[i]);
        if (node->children[i]->type == TXC_NAN)
        {
            for (size_t j = 0; j < node->children_amount; j++)
            {
                if (j == i)
                    continue;
                txc_node_free(node->children[j]);
            }
            return node->children[i];
        }
    }
    switch (node->type)
    {
    case TXC_NEG:
        switch (node->children[0]->type)
        {
        case TXC_INT:
        {
            txc_int *result = txc_int_copy_read(node->children[0]->impl.integer);
            txc_node_free(node);
            if (result == NULL)
                return (txc_node *)&TXC_NAN_ERROR_ALLOC;
            return txc_int_to_node(txc_int_neg(result));
        }
        case TXC_NEG:
        {
            txc_node *result = txc_node_copy_read(node->children[0]->children[0]);
            txc_node_free(node);
            return result;
        }
        // TODO
        default:
            return node;
        }
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL:
    {
        txc_node *const copy = txc_node_copy_write(node);
        if (copy->type == TXC_NAN)
        {
            txc_node_free(copy);
            return node;
        }
        assert(copy->read_only == false);
        copy->children_amount = 0;
        for (size_t i = 0; i < node->children_amount; i++)
        {
            if (node->children[i]->type == node->type)
                copy->children_amount += node->children[i]->children_amount;
            else
                copy->children_amount++;
        }
        free(copy->children);
        copy->children = malloc(sizeof *copy->children * copy->children_amount);
        if (copy->children == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, sizeof copy->children * copy->children_amount, "compact copy", __FILE__, __LINE__);
            copy->children_amount = 0;
            txc_node_free(copy);
            return node;
        }
        size_t ii = 0;
        for (size_t j = 0; ii < copy->children_amount; j++)
        {
            if (node->children[j]->type != copy->type)
            {
                copy->children[ii] = txc_node_copy_read(node->children[j]);
                ii++;
            }
            else
            {
                for (size_t k = 0; k < node->children[j]->children_amount && ii < copy->children_amount; k++)
                {
                    copy->children[ii] = txc_node_copy_read(node->children[j]->children[k]);
                    ii++;
                }
            }
        }
        txc_node_free(node);
        // TODO debug print if top level?
        size_t int_i = 0;
        size_t other_i = 0;
        for (size_t i = 0; i < copy->children_amount; i++)
            if (copy->children[i]->type == TXC_INT)
                int_i++;
        if (int_i <= 0)
            return copy;
        const txc_int *ints[int_i];
        txc_node *int_nodes[int_i];
        int_i = 0;
        for (size_t i = 0; i < copy->children_amount; i++)
        {
            if (copy->children[i]->type == TXC_INT)
            {
                ints[int_i] = txc_node_to_int(copy->children[i]);
                int_nodes[int_i] = copy->children[i];
                int_i++;
            }
            else
            {
                copy->children[other_i] = copy->children[i];
                other_i++;
            }
        }
        copy->children_amount = other_i + 1;
        struct txc_node **tmp = realloc(copy->children, sizeof *tmp * copy->children_amount);
        if (tmp == NULL)
        {
            txc_node_free(copy);
            fprintf(stderr, TXC_ERROR_ALLOC, sizeof tmp * copy->children_amount, "others only children", __FILE__, __LINE__);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        copy->children = tmp;
        txc_int *integer;
        switch (copy->type)
        {
        case TXC_ADD:
            integer = txc_int_add(ints, int_i);
            break;
        case TXC_MUL:
            integer = txc_int_mul(ints, int_i);
            break;
        default:
            for (size_t i = 0; i < int_i; i++)
                txc_node_free(int_nodes[i]);
            copy->children_amount--;
            txc_node_free(copy);
            fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, copy->type, __FILE__, __LINE__);
            return (txc_node *)&TXC_NAN_ERROR_INVALID_NODE_TYPE;
        }
        for (size_t i = 0; i < int_i; i++)
            txc_node_free(int_nodes[i]);
        if (integer == NULL)
        {
            copy->children_amount--;
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        txc_node *int_node = txc_int_to_node(integer);
        if (int_node == NULL)
        {
            copy->children_amount--;
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        if (copy->children_amount <= 1)
        {
            copy->children_amount--;
            txc_node_free(copy);
            return int_node;
        }
        copy->children[copy->children_amount - 1] = int_node;
        return bake(copy);
    }
    case TXC_FRAC:
    {
        if (node->children[0]->type == TXC_INT && txc_int_is_zero(node->children[0]->impl.integer))
        {
            txc_node_free(node);
            return (txc_node *)&TXC_NAN_ZERO_DIVISION;
        }
        txc_node *copy = txc_node_copy_write(node);
        bool neg = false;
        if (copy->children[0]->type == TXC_INT && txc_int_is_neg(copy->children[0]->impl.integer))
        {
            neg = true;
            copy->children[0] = txc_node_simplify(txc_node_create_un_op(TXC_NEG, copy->children[0]));
        }
        else if (copy->children[0]->type == TXC_NEG)
        {
            neg = true;
            copy->children[0] = txc_node_copy_read(node->children[0]->children[0]);
        }
        if (copy->children_amount == 1)
        {
            txc_node_free(node);
            if (txc_int_is_pos_one(copy->children[0]->impl.integer))
            {
                txc_node *const tmp = txc_node_copy_read(copy->children[0]);
                txc_node_free(copy);
                copy = tmp;
            }
            if (neg)
            {
                txc_node *const tmp = txc_node_create_un_op(TXC_NEG, copy);
                txc_node_free(copy);
                copy = tmp;
            }
            return bake(copy);
        }
        if (copy->children[1]->type == TXC_INT && txc_int_is_neg(copy->children[1]->impl.integer))
        {
            neg = !neg;
            copy->children[1] = txc_node_simplify(txc_node_create_un_op(TXC_NEG, copy->children[1]));
        }
        else if (copy->children[1]->type == TXC_NEG)
        {
            neg = !neg;
            copy->children[0] = txc_node_copy_read(node->children[0]->children[0]);
        }
        txc_node_free(node);
        if (copy->children[0]->type == TXC_INT && txc_int_is_pos_one(copy->children[0]->impl.integer))
        {
            txc_node *tmp = txc_node_copy_read(copy->children[1]);
            txc_node_free(copy);
            if (neg)
            {
                txc_node *const tmptmp = txc_node_create_un_op(TXC_NEG, txc_node_copy_read(tmp));
                txc_node_free(tmp);
                tmp = tmptmp;
            }
            return bake(txc_node_simplify(tmp));
        }
        // TODO switch if frac
        if (copy->children[0]->type == TXC_NAN)
        {
            txc_node *tmp = txc_node_copy_read(copy->children[0]);
            txc_node_free(copy);
            return tmp;
        }
        if (copy->children[1]->type == TXC_NAN)
        {
            txc_node *tmp = txc_node_copy_read(copy->children[1]);
            txc_node_free(copy);
            return tmp;
        }
        txc_int *num = NULL;
        txc_int *den = NULL;
        if (copy->children[0]->type == TXC_INT)
        {
            den = txc_int_copy_write(copy->children[0]->impl.integer);
        }
        else if (copy->children[0]->type == TXC_MUL && copy->children[0]->children[copy->children[0]->children_amount - 1]->type == TXC_INT)
        {
            den = txc_int_copy_write(copy->children[0]->children[copy->children[0]->children_amount - 1]->impl.integer);
            txc_node *tmp = txc_node_copy_write(copy->children[0]);
            if (tmp == NULL)
            {
                txc_int_free(den);
                den = NULL;
            }
            else
            {
                copy->children[0] = tmp;
            }
        }
        else
        {
            return copy;
        }
        if (den == NULL)
        {
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        if (copy->children[1]->type == TXC_INT)
        {
            num = txc_int_copy_write(copy->children[1]->impl.integer);
        }
        else if (copy->children[1]->type == TXC_MUL && copy->children[1]->children[copy->children[1]->children_amount - 1]->type == TXC_INT)
        {
            num = txc_int_copy_write(copy->children[1]->children[copy->children[1]->children_amount - 1]->impl.integer);
            txc_node *tmp = txc_node_copy_write(copy->children[1]);
            if (tmp == NULL)
            {
                txc_int_free(num);
                num = NULL;
            }
            else
            {
                copy->children[0] = tmp;
            }
        }
        else
        {
            txc_int_free(den);
            return copy;
        }
        if (num == NULL)
        {
            txc_int_free(den);
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        txc_int *gcd_den = txc_int_copy_write(den);
        if (gcd_den == NULL)
        {
            txc_int_free(den);
            txc_int_free(num);
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        txc_int *gcd_num = txc_int_copy_write(num);
        if (gcd_num == NULL)
        {
            txc_int_free(gcd_den);
            txc_int_free(den);
            txc_int_free(num);
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        txc_int *const gcd = txc_int_gcd(gcd_num, gcd_den);
        if (gcd == NULL)
        {
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        txc_int *new_den = txc_int_div_mod(den, gcd)->a;
        txc_int_free(den);
        if (new_den == NULL)
        {
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        txc_int *new_num = txc_int_div_mod(num, gcd)->a;
        txc_int_free(num);
        txc_int_free(gcd_den);
        txc_int_free(gcd_num);
        if (new_num == NULL)
        {
            txc_int_free(new_den);
            txc_node_free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        txc_node *den_node = txc_int_to_node(new_den);
        if (den_node->type == TXC_NAN)
        {
            txc_int_free(new_den);
            txc_int_free(new_num);
            txc_node_free(copy);
            return den_node;
        }
        txc_node *num_node = txc_int_to_node(new_num);
        if (num_node->type == TXC_NAN)
        {
            txc_node_free(den_node);
            txc_int_free(new_den);
            txc_int_free(new_num);
            txc_node_free(copy);
            return num_node;
        }
        den_node = bake(den_node);
        num_node = bake(num_node);
        if (copy->children[0]->type == TXC_INT)
        {
            txc_node_free(copy->children[0]);
            copy->children[0] = den_node;
        }
        else
        {
            txc_node_free(copy->children[0]->children[copy->children[0]->children_amount - 1]);
            copy->children[0]->children[copy->children[0]->children_amount - 1] = den_node;
            copy->children[0] = bake(txc_node_simplify(copy->children[0]));
        }
        if (copy->children[1]->type == TXC_INT)
        {
            txc_node_free(copy->children[1]);
            copy->children[1] = num_node;
        }
        else
        {
            txc_node_free(copy->children[1]->children[copy->children[1]->children_amount - 1]);
            copy->children[1]->children[copy->children[1]->children_amount - 1] = num_node;
            copy->children[1] = bake(txc_node_simplify(copy->children[1]));
        }
        if (copy->children[0]->type == TXC_INT && txc_int_is_pos_one(copy->children[0]->impl.integer))
        {
            txc_node *tmp = txc_node_copy_read(copy->children[1]);
            txc_node_free(copy);
            if (!neg)
                return tmp;
            txc_node *tmptmp = txc_node_create_un_op(TXC_NEG, tmp);
            if (tmptmp == NULL)
            {
                txc_node_free(tmp);
                return (txc_node *)&TXC_NAN_ERROR_ALLOC;
            }
            return txc_node_simplify(tmptmp);
        }
        if (copy->children[1]->type == TXC_INT && txc_int_is_pos_one(copy->children[1]->impl.integer))
        {
            struct txc_node **tmp = realloc(copy->children, sizeof *copy->children * 1);
            if (tmp != NULL)
            {
                txc_node_free(copy->children[1]);
                copy->children = tmp;
                copy->children_amount = 1;
            }
        }
        if (neg)
        {
            txc_node *tmp = txc_node_create_un_op(TXC_NEG, copy);
            if (tmp == NULL)
            {
                txc_node_free(copy);
                return (txc_node *)&TXC_NAN_ERROR_ALLOC;
            }
            return txc_node_simplify(tmp);
        }
        return bake(copy);
    }
    case TXC_NAN: /* FALLTHROUGH */
    case TXC_INT: /* FALLTHROUGH */
    default:
        return node;
    }
}

/* PRINT */

static char *concat_children_in_paren(struct txc_node **const children, const size_t amount, const bool reverse_children, const char *const pre, const char *const op, const char *const post)
{
    size_t size = strlen(pre) + strlen(op) * (amount - 1) + strlen(post);
    char *child_strs[amount];
    for (size_t i = 0; i < amount; i++)
    {
        child_strs[i] = txc_node_to_str(children[reverse_children ? amount - i - 1 : i]);
        if (child_strs[i] == NULL)
        {
            for (size_t j = 0; j < i; j++)
                free(child_strs[j]);
            return NULL;
        }
        size += strlen(child_strs[i]);
    }
    char *const buf = malloc(sizeof *buf * (size + 1));
    if (buf == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, size, "concatinated string", __FILE__, __LINE__);
        for (size_t i = 0; i < amount; i++)
            free(child_strs[i]);
        return txc_node_to_str((txc_node *)&TXC_NAN_ERROR_ALLOC);
    }
    char *cur = txc_stpcpy(buf, pre);
    for (size_t i = 0; i < amount; i++)
    {
        if (i > 0)
            cur = txc_stpcpy(cur, op);
        cur = txc_stpcpy(cur, child_strs[i]);
    }
    txc_stpcpy(cur, post);
    for (size_t i = 0; i < amount; i++)
        free(child_strs[i]);
    return buf;
}

char *txc_node_to_str(const txc_node *const node)
{
    node_assert_valid(node);
    char *str = NULL;
    switch (node->type)
    {
    case TXC_NAN:
    {
        size_t size = sizeof str * (strlen(TXC_NAN_REASON) + strlen(node->impl.reason) + 1);
        str = malloc(size);
        if (str == NULL)
            fprintf(stderr, TXC_ERROR_ALLOC, size, "nan string", __FILE__, __LINE__);
        else
            snprintf(str, size, TXC_NAN_REASON, node->impl.reason);
        break;
    }
    case TXC_INT:
        str = (char *)txc_int_to_str(node->impl.integer);
        if (str == NULL)
            str = txc_node_to_str((txc_node *)&TXC_NAN_ERROR_ALLOC);
        break;
    case TXC_NEG:
        str = concat_children_in_paren(node->children, node->children_amount, false, "(-", "", ")");
        break;
    case TXC_ADD:
        str = concat_children_in_paren(node->children, node->children_amount, false, "(", " + ", ")");
        break;
    case TXC_MUL:
        str = concat_children_in_paren(node->children, node->children_amount, false, "(", " \\cdot ", ")");
        break;
    case TXC_FRAC:
        if (node->children_amount == 1)
            str = concat_children_in_paren(node->children, node->children_amount, true, "\\frac{1}{", "", "}");
        else
            str = concat_children_in_paren(node->children, node->children_amount, true, "\\frac{", "}{", "}");
        break;
    default:
        fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, node->type, __FILE__, __LINE__);
        break;
    }
    return str;
}

void txc_node_print(txc_node *const node)
{
    node_assert_valid(node);
    char *str = txc_node_to_str(node);
    if (str == NULL)
    {
        printf(TXC_PRINT_FORMAT, TXC_PRINT_ERROR);
    }
    else
    {
        printf(TXC_PRINT_FORMAT, str);
        free(str);
    }
}

void txc_node_print_if_debug(txc_node *const node)
{
    node_assert_valid(node);
#if DEBUG
    txc_node_print(node);
#endif /* DEBUG */
}

void txc_node_simplify_and_print(txc_node *const node)
{
    node_assert_valid(node);
    txc_node_print_if_debug(node);
    txc_node *const simple_node = txc_node_simplify(node);
    txc_node_print(simple_node);
}
