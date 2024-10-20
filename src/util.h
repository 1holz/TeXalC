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

#ifndef TXC_UTIL
#define TXC_UTIL

#include "common.h"

#include <stdbool.h>

#define TXC_ERROR_ALLOC(bytes, purpose) fprintf(stderr, "Allocating %zu bytes of memory failed for %s at %s:%u.\n", bytes, purpose, __FILE__, __LINE__)
#define TXC_ERROR_INVALID_CHILD_AMOUNT(type, amount) fprintf(stderr, "Node of type %u must not have %zu children at %s:%u.\n", type, amount, __FILE__, __LINE__)
#define TXC_ERROR_INVALID_NODE_TYPE(type) fprintf(stderr, "Node type %u is invalid at %s:%u.\n", type, __FILE__, __LINE__)
#define TXC_ERROR_INT_LEADING_ZERO() fprintf(stderr, "Integer must not have leading zero bytes at %s:%u.\n", __FILE__, __LINE__)
#define TXC_ERROR_NULL(object) fprintf(stderr, "NULL is not allowed for %s at %s:%u.\n", object, __FILE__, __LINE__)
#define TXC_ERROR_NYI(feature) fprintf(stderr, "%s is/are not yet implemented at %s:%u.\n", feature, __FILE__, __LINE__)
#define TXC_ERROR_OVERFLOW(cause) fprintf(stderr, "Overflow was caught for %s in %s line %u.\n", cause, __FILE__, __LINE__)
#define TXC_ERROR_OUT_OF_BOUNDS(index, bound) fprintf(stderr, "Index %zu out of bound %zu in %s line %u.\n", index, bound, __FILE__, __LINE__)

struct txc_size_t_tuple {
    size_t a;
    size_t b;
};

struct txc_char_ptr_tuple {
    char *a;
    char *b;
};

typedef struct txc_int txc_int;

enum txc_node_type {
    TXC_NAN,
    TXC_INT,
    TXC_NEG,
    TXC_ADD,
    TXC_MUL,
    TXC_FRAC
};

// TODO mmissing txc prefix
union impl {
    char *reason;
    struct txc_int *integer;
};

struct txc_node {
    union impl impl;
    size_t children_amount;
    size_t gc_i;
    enum txc_node_type type;
    bool read_only;
    struct txc_node *children[];
};

typedef struct txc_mem_gc txc_mem_gc;

extern char *txc_strdup(const char *const str);

extern char *txc_strrev(char *const str);

extern char *txc_stpcpy(char *restrict dst, const char *restrict src);

#endif /* TXC_UTIL */
