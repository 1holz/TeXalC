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
#include "numbers.h"
#include "util.h"

#define TXC_ERROR_NYI "Not yet implemented in %s line %d.\n"
#define TXC_ERROR_ILLEGAL_ARG "Illegal argument %s in %s line %d.\n"
#define TXC_ERROR_ALLOC "Could not allocate %zu bytes of memory for %s in %s line %d.\n"
#define TXC_ERROR_INVALID_NODE_TYPE "Number type %du is invalid in %s line %d.\n"

#define TXC_NAN_REASON "\\text{NAN(%s)}"
#define TXC_NAN_REASON_ERROR_ALLOC "Could not allocate enough memory. Please see stderr for more information."
#define TXC_NAN_REASON_ERROR_NYI "Not yet implemented. Please see stderr for more information."
#define TXC_NAN_REASON_ERROR_INVALID_NODE_TYPE "Number type is invalid. Please see stderr for more information."
#define TXC_NAN_REASON_UNSPECIFIED "unspecified"

#define TXC_PRINT_FORMAT "= %s \\\\\n\n"
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
const txc_node TXC_NAN_ERROR_NYI = {.children = NULL,
                                    .impl.reason = TXC_NAN_REASON_ERROR_NYI,
                                    .children_amount = 0,
                                    .type = TXC_NAN,
                                    .read_only = true};
const txc_node TXC_NAN_ERROR_INVALID_NODE_TYPE = {.children = NULL,
                                                  .impl.reason = TXC_NAN_REASON_ERROR_INVALID_NODE_TYPE,
                                                  .children_amount = 0,
                                                  .type = TXC_NAN,
                                                  .read_only = true};
const txc_node TXC_NAN_UNSPECIFIED = {.children = NULL,
                                      .impl.reason = TXC_NAN_REASON_UNSPECIFIED,
                                      .children_amount = 0,
                                      .type = TXC_NAN,
                                      .read_only = true};

/* ASSERTS */

static void node_assert_valid(const struct txc_node *const node)
{
    assert(node->children_amount == 0 || node->children != NULL);
}

/* CREATE, COPY AND FREE */

txc_node *txc_create_nan(const char *const reason)
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
    return nan;
}

txc_node *txc_create_bin_op(const enum txc_node_type type, txc_node *const operand_1, txc_node *const operand_2)
{
    if (type != TXC_ADD && type != TXC_MUL)
    {

        fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, type, __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_INVALID_NODE_TYPE;
    }
    txc_node *node = malloc(sizeof *node);
    if (node == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *node, "binary operation node", __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    node->children_amount = 2;
    node->children = malloc(sizeof *node->children * node->children_amount);
    if (node->children == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof node->children * node->children_amount, "binary operation node children", __FILE__, __LINE__);
        free(node);
        return (txc_node *)&TXC_NAN_ERROR_ALLOC;
    }
    node->children[0] = operand_1;
    node->children[1] = operand_2;
    node->type = type;
    node->read_only = false;
    return node;
}

txc_node *txc_node_create(struct txc_node **children, union impl impl, size_t children_amount, enum txc_node_type type)
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
    return node;
}

txc_node *txc_node_copy(txc_node *const from)
{
    node_assert_valid(from);
    if (from->read_only)
        return from;
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
            copy->children[i] = txc_node_copy(from->children[i]);
    }
    switch (copy->type)
    {
    case TXC_NAN:
        copy->impl.reason = txc_strdup(from->impl.reason);
        if (copy->impl.reason == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, sizeof *copy, "copy reason", __FILE__, __LINE__);
            for (size_t i = 0; i < copy->children_amount; i++)
                txc_node_free(from->children[i]);
            free(copy->children);
            free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        break;
    case TXC_NUM:
        copy->impl.natural_num = txc_copy_num(from->impl.natural_num);
        if (copy->impl.natural_num == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, sizeof *copy, "copy natural number", __FILE__, __LINE__);
            free(copy);
            return (txc_node *)&TXC_NAN_ERROR_ALLOC;
        }
        break;
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL: /* FALLTHROUGH */
    default:
        break;
    }
    return copy;
}

void txc_node_free(txc_node *const node)
{
    node_assert_valid(node);
    if (node->read_only)
        return;
    for (size_t i = 0; i < node->children_amount; i++)
        txc_node_free(node->children[i]);
    free(node->children);
    switch (node->type)
    {
    case TXC_NAN:
        free(node->impl.reason);
        break;
    case TXC_NUM:
        txc_free_num(node->impl.natural_num);
        break;
    case TXC_ADD:
        free(node->impl.reason);
        break;
    case TXC_MUL:
        free(node->impl.reason);
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
    case TXC_ADD: /* FALLTHROUGH */
    case TXC_MUL:
    {
        txc_node *const copy = txc_node_copy(node);
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
        size_t j = 0;
        for (size_t i = 0; i < copy->children_amount; i++)
        {
            if (node->children[j]->type != copy->type)
            {
                copy->children[i] = node->children[j];
                j++;
                continue;
            }
            for (size_t k = 0; k < node->children[j]->children_amount && i < copy->children_amount; k++)
            {
                copy->children[i] = txc_node_copy(node->children[j]->children[k]);
                i++;
            }
            txc_node_free(node->children[j]);
        }
        txc_node_free(node);
        switch (copy->type)
        {
        case TXC_ADD:
            txc_node_free(copy);
            fprintf(stderr, TXC_ERROR_NYI, __FILE__, __LINE__);
            return (txc_node *)&TXC_NAN_ERROR_NYI;
        case TXC_MUL:
            txc_node_free(copy);
            fprintf(stderr, TXC_ERROR_NYI, __FILE__, __LINE__);
            return (txc_node *)&TXC_NAN_ERROR_NYI;
        default:
            txc_node_free(copy);
            fprintf(stderr, TXC_ERROR_INVALID_NODE_TYPE, copy->type, __FILE__, __LINE__);
            return (txc_node *)&TXC_NAN_ERROR_INVALID_NODE_TYPE;
        }
        return node;
    }
    case TXC_NAN: /* FALLTHROUGH */
    case TXC_NUM: /* FALLTHROUGH */
    default:
        return node;
    }
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
    case TXC_NUM:
        str = (char *)txc_num_to_str(node->impl.natural_num);
        if (str == NULL)
            str = txc_node_to_str((txc_node *)&TXC_NAN_ERROR_ALLOC);
        break;
    case TXC_ADD:
        str = txc_node_to_str((txc_node *)&TXC_NAN_ERROR_NYI);
        break;
    case TXC_MUL:
        str = txc_node_to_str((txc_node *)&TXC_NAN_ERROR_NYI);
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
    char *const str = txc_node_to_str(node);
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
