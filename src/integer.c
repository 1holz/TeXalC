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

#include "common.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "integer.h"
#include "util.h"

#ifndef TXC_INT_GROWTH_FACTOR
#define TXC_INT_GROWTH_FACTOR 2
#endif /* TXC_INT_GROWTH_FACTOR */

#ifndef TXC_INT_ARRAY_TYPE
#define TXC_INT_ARRAY_TYPE unsigned char
#endif /* TXC_INT_ARRAY_TYPE */
#ifndef TXC_INT_ARRAY_TYPE_MAX
#define TXC_INT_ARRAY_TYPE_MAX UCHAR_MAX
#endif /* TXC_INT_ARRAY_TYPE_MAX */
#ifndef TXC_INT_ARRAY_TYPE_WIDTH
#define TXC_INT_ARRAY_TYPE_WIDTH CHAR_BIT
#endif /* TXC_INT_ARRAY_TYPE_WIDTH */

/* DEFINITIONS */

struct txc_int {
    size_t size;
    size_t used;
    bool neg;
    TXC_INT_ARRAY_TYPE data[];
};

/* VALID */

bool txc_int_test_valid(const struct txc_int *const integer)
{
    if (integer == NULL) {
        TXC_ERROR_NULL("txc_int");
        return false;
    }
    if (integer->used > integer->size) {
        TXC_ERROR_OUT_OF_BOUNDS(integer->used, integer->size);
        return false;
    }
    if (integer->used > 0 && integer->data[integer->used - 1] <= 0) {
        TXC_ERROR_INT_LEADING_ZERO();
        return false;
    }
    return true;
}

/* MEMORY */

const txc_node *txc_int_to_node(const struct txc_int *const integer)
{
    if (integer == NULL)
        return &TXC_NAN_ERROR_ALLOC; // TODO other causes for NULL?
    assert(txc_int_test_valid(integer));
    union impl impl;
    impl.integer = (struct txc_int *)integer;
    return txc_node_create(NULL, impl, 0, TXC_INT);
}

static struct txc_int *init(const size_t size)
{
    struct txc_int *const integer = malloc(sizeof *integer + sizeof *integer->data * size);
    if (integer == NULL) {
        TXC_ERROR_ALLOC(sizeof *integer + sizeof *integer->data * size, "int");
        return NULL;
    }
    integer->size = size;
    integer->used = 0;
    integer->neg = false;
    return integer;
}

static struct txc_int *inc_size(struct txc_int *const integer, size_t new_size)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    if (integer->used >= SIZE_MAX)
        return integer;
    const size_t max_size = SIZE_MAX / sizeof *integer->data - sizeof *integer;
    if (new_size >= max_size)
        new_size = max_size;
    if (new_size <= integer->size)
        return integer;
    struct txc_int *tmp = realloc(integer, sizeof *integer + sizeof *integer->data * new_size);
    if (tmp == NULL) {
        txc_int_free(integer);
        TXC_ERROR_ALLOC(sizeof *integer + sizeof *integer->data * new_size, "new size");
        return NULL;
    }
    tmp->size = new_size;
    return tmp;
}

static struct txc_int *inc(struct txc_int *const integer)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    size_t new_size = integer->used * TXC_INT_GROWTH_FACTOR;
    if (new_size <= integer->used)
        new_size = integer->used++;
    return inc_size(integer, new_size);
}

static struct txc_int *fit(struct txc_int *const integer)
{
    if (integer == NULL)
        return NULL;
    while (integer->used > 0 && integer->data[integer->used - 1] == 0)
        integer->used--;
    assert(txc_int_test_valid(integer));
    if (integer->used == 0)
        integer->neg = false;
    if (integer->size == integer->used)
        return integer;
    struct txc_int *const fitted = realloc(integer, sizeof *integer + sizeof *integer->data * integer->used);
    if (fitted == NULL) {
        TXC_ERROR_ALLOC(sizeof *integer + sizeof *integer->data * integer->used, "fitted integer");
        return integer;
    }
    fitted->size = fitted->used;
    return fitted;
}

static struct txc_int *from_bin_str(struct txc_int *const integer, const char *const str, const size_t len)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    for (size_t i = 0; i < len; i++)
        assert('0' <= str[i] && str[i] <= '1');
    const size_t bin_width = 1;
    const size_t chars_per_elem = TXC_INT_ARRAY_TYPE_WIDTH / bin_width;
    for (size_t i = 0; i < len; i++) {
        if (i % chars_per_elem == 0) {
            integer->data[integer->used] = 0;
            integer->used++;
        }
        integer->data[integer->used - 1] += (str[len - i - 1] - 48) << ((i % chars_per_elem) * bin_width);
    }
    return integer;
}

static struct txc_int *from_dec_str(struct txc_int *const integer, const char *const str, const size_t len)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    for (size_t i = 0; i < len; i++)
        assert('0' <= str[i] && str[i] <= '9');
    const size_t dec_width = 4;
    unsigned char buf[len];
    for (size_t i = 0; i < len; i++)
        buf[i] = str[len - i - 1] - 48;
    integer->data[0] = 0;
    integer->used = 1;
    uint_fast8_t bit_i = 0;
    size_t buf_len = len;
    while (buf_len > 0) {
        if (buf[buf_len - 1] == 0) {
            buf_len--;
            continue;
        }
        bool carry = false;
        for (size_t ii = 0; ii < buf_len; ii++) {
            size_t i = buf_len - ii - 1;
            bool new_carry = buf[i] % 2 == 1;
            buf[i] >>= 1;
            if (carry)
                buf[i] += 1 << (dec_width - 1);
            if (buf[i] >= 8)
                buf[i] -= 3;
            carry = new_carry;
        }
        if (carry)
            integer->data[integer->used - 1] += 1 << bit_i;
        if (bit_i >= TXC_INT_ARRAY_TYPE_WIDTH - 1) {
            integer->data[integer->used] = 0;
            integer->used++;
            bit_i = 0;
        } else {
            bit_i++;
        }
    }
    return integer;
}

static struct txc_int *from_hex_str(struct txc_int *const integer, const char *const str, const size_t len)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    for (size_t i = 0; i < len; i++)
        assert(('0' <= str[i] && str[i] <= '9') || ('A' <= str[i] && str[i] <= 'F') || ('a' <= str[i] && str[i] <= 'f'));
    const size_t hex_width = 4;
    const size_t chars_per_elem = TXC_INT_ARRAY_TYPE_WIDTH / hex_width;
    for (size_t i = 0; i < len; i++) {
        if (i % chars_per_elem == 0) {
            integer->data[integer->used] = 0;
            integer->used++;
        }
        const char buf[2] = { str[len - i - 1], 0 };
        integer->data[integer->used - 1] += strtoul((const char *restrict)&buf, NULL, 16) << ((i % chars_per_elem) * hex_width);
    }
    return integer;
}

const struct txc_node *txc_int_create_int_node(const char *const str, size_t len, const uint_fast8_t base)
{
    if (base != 2 && base != 10 && base != 16) {
        TXC_ERROR_NYI("Bases other than 2, 10 or 16");
        return &TXC_NAN_ERROR_NYI;
    }
    const char *cur = str;
    size_t width = 4;
    if (base == 2)
        width = 1;
    for (; cur[0] == '0'; len--) // potencial buffer overflow when only 0 though this should be caught by lexer already
        cur++;
    if (len <= 0)
        return txc_int_to_node(txc_int_create_zero());
    size_t chars_per_elem = TXC_INT_ARRAY_TYPE_WIDTH / width;
    txc_int *integer = init(len / chars_per_elem + 1);
    if (integer == NULL)
        return NULL;
    switch (base) {
    case 2:
        integer = from_bin_str(integer, cur, len);
        break;
    default: /* FALLTHROUGH */
    case 10:
        integer = from_dec_str(integer, cur, len);
        break;
    case 16:
        integer = from_hex_str(integer, cur, len);
        break;
    }
    return txc_int_to_node(fit(integer));
}

struct txc_int *txc_int_create_zero(void)
{
    struct txc_int *zero = init(0);
    if (zero == NULL)
        return NULL;
    return zero;
}

struct txc_int *txc_int_create_one(void)
{
    struct txc_int *one = init(1);
    if (one == NULL)
        return NULL;
    one->used = 1;
    one->data[0] = 1;
    return one;
}

struct txc_int *txc_int_copy(const struct txc_int *const from)
{
    if (from == NULL)
        return NULL;
    assert(txc_int_test_valid(from));
    struct txc_int *copy = init(from->used);
    if (copy == NULL)
        return NULL;
    copy->used = from->used;
    copy->size = from->used;
    copy->neg = from->neg;
    for (size_t i = 0; i < copy->used; i++)
        copy->data[i] = from->data[i];
    return copy;
}

void txc_int_free(const struct txc_int *const integer)
{
    if (integer == NULL)
        return;
    assert(txc_int_test_valid(integer));
    free((struct txc_int *)integer);
}

/* INTEGER */

bool txc_int_is_pos_one(const struct txc_int *const test)
{
    if (test == NULL)
        return false;
    assert(txc_int_test_valid(test));
    return test->used == 1 && test->neg == false && test->data[0] == 1;
}

bool txc_int_is_zero(const struct txc_int *const test)
{
    if (test == NULL)
        return false;
    assert(txc_int_test_valid(test));
    return test->used == 0;
}

bool txc_int_is_neg_one(const struct txc_int *const test)
{
    if (test == NULL)
        return false;
    assert(txc_int_test_valid(test));
    return test->used == 1 && test->neg == true && test->data[0] == 1;
}

bool txc_int_is_neg(const struct txc_int *const test)
{
    if (test == NULL)
        return false;
    assert(txc_int_test_valid(test));
    return test->neg == true && !txc_int_is_zero(test);
}

static struct txc_int *shift_bigger(struct txc_int *integer)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    if (integer->used >= integer->size && integer->data[integer->used - 1] > TXC_INT_ARRAY_TYPE_MAX / 2) {
        integer = inc(integer);
        if (integer == NULL)
            return NULL;
    }
    bool carry = false;
    for (size_t i = 0; i < integer->used; i++) {
        bool new_carry = integer->data[i] > TXC_INT_ARRAY_TYPE_MAX / 2;
        integer->data[i] <<= 1;
        if (carry)
            integer->data[i] += 1;
        carry = new_carry;
    }
    if (carry) {
        integer->data[integer->used] = 1;
        integer->used++;
    }
    return integer;
}

static struct txc_int *shift_bigger_amount(struct txc_int *integer, const size_t amount)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    for (size_t i = 0; i < amount; i++)
        integer = shift_bigger(integer);
    return integer;
}

static struct txc_int *shift_bigger_wide_amount(struct txc_int *integer, const size_t bytes, const size_t bits)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    integer = inc_size(integer, integer->used + bytes);
    if (integer == NULL)
        return NULL;
    for (size_t i = 0; i < integer->used; i++)
        integer->data[bytes + integer->used - i - 1] = integer->data[integer->used - i - 1];
    for (size_t i = 0; i < bytes; i++)
        integer->data[i] = 0;
    integer->used = integer->used + bytes;
    return shift_bigger_amount(integer, bits);
}

static struct txc_int *shift_smaller(struct txc_int *const integer)
{
    assert(txc_int_test_valid(integer));
    if (txc_int_is_zero(integer))
        return integer;
    for (size_t i = 0; i < integer->used - 1; i++) {
        integer->data[i] >>= 1;
        if (integer->data[i + 1] % 2 > 0)
            integer->data[i] += TXC_INT_ARRAY_TYPE_MAX / 2 + 1;
    }
    integer->data[integer->used - 1] >>= 1;
    if (integer->data[integer->used - 1] == 0) {
        integer->used--;
        if (integer->used <= 0)
            integer->neg = false;
    }
    return integer;
}

static struct txc_int *shift_smaller_amount(struct txc_int *integer, size_t amount)
{
    assert(txc_int_test_valid(integer));
    for (size_t i = 0; i < amount && integer->used > 0; i++)
        integer = shift_smaller(integer);
    return integer;
}

static struct txc_int *shift_smaller_wide_amount(struct txc_int *const integer, size_t bytes, const size_t bits)
{
    assert(txc_int_test_valid(integer));
    if (bytes > integer->used)
        bytes = integer->used;
    integer->used = integer->used - bytes;
    for (size_t i = 0; i < integer->used; i++)
        integer->data[i] = integer->data[i + bytes];
    return shift_smaller_amount(integer, bits);
}

int_fast8_t txc_int_cmp_abs(const struct txc_int *const a, const struct txc_int *const b)
{
    assert(txc_int_test_valid(a));
    assert(txc_int_test_valid(b));
    if (a->used == 0 && b->used == 0)
        return 0;
    if (a->used < b->used)
        return -1;
    if (a->used > b->used)
        return 1;
    const size_t used = a->used;
    for (size_t i = 0; i < used; i++) {
        if (a->data[used - i - 1] < b->data[used - i - 1])
            return -1;
        if (a->data[used - i - 1] > b->data[used - i - 1])
            return 1;
    }
    return 0;
}

int_fast8_t txc_int_cmp(const struct txc_int *const a, const struct txc_int *const b)
{
    assert(txc_int_test_valid(a));
    assert(txc_int_test_valid(b));
    if (a->used == 0 && b->used == 0)
        return 0;
    if (a->neg && !b->neg)
        return 1;
    if (!a->neg && b->neg)
        return -1;
    return txc_int_cmp_abs(a, b) * (a->neg && b->neg ? -1 : 1);
}

struct txc_int *txc_int_neg(struct txc_int *const integer)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    integer->neg = !integer->neg;
    return integer;
}

static struct txc_int *add_acc(struct txc_int *acc, const struct txc_int *const summand)
{
    if (acc == NULL)
        return NULL;
    if (summand == NULL) {
        txc_int_free(acc);
        return NULL;
    }
    assert(txc_int_test_valid(acc));
    assert(txc_int_test_valid(summand));
    bool carry = false;
    if (acc->neg != summand->neg) {
        const int_fast8_t abs_cmp = txc_int_cmp_abs(acc, summand);
        if (abs_cmp == 0) {
            txc_int_free(acc);
            return txc_int_create_zero();
        }
        struct txc_int *smaller = acc;
        if (abs_cmp < 0)
            acc = txc_int_copy(summand);
        else
            smaller = txc_int_copy(summand);
        if (acc == NULL || smaller == NULL) {
            txc_int_free(acc);
            return NULL;
        }
        for (size_t i = 0; i < acc->used && (i < smaller->used || carry); i++) {
            if (carry) {
                carry = acc->data[i] <= 0;
                acc->data[i]--;
            }
            if (i < smaller->used) {
                if (acc->data[i] < smaller->data[i])
                    carry = true;
                acc->data[i] -= smaller->data[i];
            }
        }
        txc_int_free(smaller);
        while (acc->data[acc->used - 1] == 0)
            acc->used--;
    } else {
        size_t result_size = summand->used > acc->used ? summand->used : acc->used;
        acc = inc_size(acc, result_size + 1);
        if (acc == NULL)
            return NULL;
        for (size_t i = 0; i < result_size && (i < summand->used || carry); i++) {
            if (i >= acc->used) {
                acc->data[acc->used] = 0;
                acc->used++;
            }
            if (carry) {
                carry = acc->data[i] >= TXC_INT_ARRAY_TYPE_MAX;
                acc->data[i]++;
            }
            if (i < summand->used) {
                if (TXC_INT_ARRAY_TYPE_MAX - acc->data[i] < summand->data[i])
                    carry = true;
                acc->data[i] += summand->data[i];
            }
        }
        if (carry) {
            acc->data[acc->used] = 1;
            acc->used++;
        }
    }
    return acc;
}

struct txc_int *txc_int_add(const struct txc_int *const *const summands, const size_t len)
{
    if (len <= 0)
        return txc_int_create_zero();
    if (summands == NULL)
        return NULL;
    assert(summands != NULL);
    for (size_t i = 0; i < len; i++) {
        if (summands[i] == NULL)
            return NULL;
        assert(txc_int_test_valid(summands[i]));
    }
    struct txc_int *acc = txc_int_create_zero();
    for (size_t i = 0; i < len; i++) {
        if (!txc_int_is_zero(summands[i]))
            acc = add_acc(acc, summands[i]);
    }
    return acc == NULL ? acc : fit(acc);
}

struct txc_int *txc_int_mul(const struct txc_int *const *const factors, const size_t len)
{
    if (len <= 0)
        return txc_int_create_one();
    if (factors == NULL)
        return NULL;
    for (size_t i = 0; i < len; i++) {
        if (factors[i] == NULL)
            return NULL;
        assert(txc_int_test_valid(factors[i]));
    }
    for (size_t i = 0; i < len; i++) {
        if (txc_int_is_zero(factors[i]))
            return txc_int_create_zero();
    }
    bool neg = factors[0]->neg;
    struct txc_int *acc = txc_int_copy(factors[0]);
    if (acc == NULL)
        return NULL;
    for (size_t i = 1; i < len; i++) {
        if (factors[i]->neg)
            neg = !neg;
        size_t result_size = acc->used + factors[i]->used;
        acc = inc_size(acc, result_size);
        if (acc == NULL)
            return NULL;
        size_t summand_i = 0;
        for (size_t j = 0; j < factors[i]->used; j++)
            for (size_t k = 0; k < TXC_INT_ARRAY_TYPE_WIDTH; k++)
                if (((factors[i]->data[j] >> k) & 1) != 0)
                    summand_i++;
        struct txc_int *summands[summand_i];
        summand_i = 0;
        for (size_t j = 0; j < factors[i]->used; j++) {
            for (size_t k = 0; k < TXC_INT_ARRAY_TYPE_WIDTH; k++) {
                if (((factors[i]->data[j] >> k) & 1) == 0)
                    continue;
                summands[summand_i] = shift_bigger_wide_amount(txc_int_copy(acc), j, k);
                summand_i++;
            }
        }
        txc_int_free(acc);
        acc = txc_int_add((const struct txc_int **)summands, summand_i);
        for (size_t j = 0; j < summand_i; j++)
            txc_int_free(summands[j]);
    }
    if (acc == NULL)
        return NULL;
    acc->neg = neg;
    return fit(acc);
}

static struct txc_size_t_tuple int_ffs(const struct txc_int *const integer)
{
    if (integer == NULL) {
        const struct txc_size_t_tuple result = { .a = 0, .b = 0 };
        return result;
    }
    assert(txc_int_test_valid(integer));
    for (size_t i = 0; i < integer->used; i++) {
        if (integer->data[i] == 0)
            continue;
        for (size_t j = 0; j < TXC_INT_ARRAY_TYPE_WIDTH; j++) {
            if (((integer->data[i] >> j) & 1) != 1)
                continue;
            const struct txc_size_t_tuple result = { .a = i, .b = j };
            return result;
        }
    }
    const struct txc_size_t_tuple result = { .a = 0, .b = 0 };
    return result;
}

// based on https://de.wikipedia.org/wiki/Steinscher_Algorithmus
struct txc_int *txc_int_gcd(const struct txc_int *const aa, const struct txc_int *const bb)
{
    if (aa == NULL || bb == NULL)
        return NULL;
    assert(txc_int_test_valid(aa));
    assert(txc_int_test_valid(bb));
    if (txc_int_is_zero(aa))
        return txc_int_copy(bb);
    if (txc_int_is_zero(bb))
        return txc_int_copy(aa);
    const size_t large_used = bb->used > aa->used ? bb->used : aa->used;
    struct txc_int *const large = txc_int_copy(bb->used > aa->used ? bb : aa);
    struct txc_int *a = inc_size(txc_int_copy(aa), large_used);
    struct txc_int *b = inc_size(txc_int_copy(bb), large_used);
    if (large == NULL || a == NULL || b == NULL) {
        txc_int_free(large);
        txc_int_free(a);
        txc_int_free(b);
        return NULL;
    }
    a->neg = false;
    b->neg = false;
    struct txc_int *small = b->used > a->used ? a : b;
    for (size_t i = 0; i < small->used; i++)
        large->data[i] = large->data[i] | small->data[i];
    const struct txc_size_t_tuple c = int_ffs(large);
    txc_int_free(large);
    const struct txc_size_t_tuple a_shift = int_ffs(a);
    a = shift_smaller_wide_amount(a, a_shift.a, a_shift.b);
    do {
        const struct txc_size_t_tuple b_shift = int_ffs(b);
        b = shift_smaller_wide_amount(b, b_shift.a, b_shift.b);
        if (txc_int_cmp_abs(a, b) > 0) {
            struct txc_int *tmp = b;
            b = a;
            a = tmp;
        }
        a->neg = true;
        b = add_acc(b, a);
        a->neg = false;
    } while (!txc_int_is_zero(b));
    a = shift_bigger_wide_amount(a, c.a, c.b);
    txc_int_free(b);
    return fit(a);
}

// FIXME calculates remainder instead of mod
static struct txc_int *div_mod(const struct txc_int *const dividend, const struct txc_int *const divisor, const bool do_mod)
{
    if (dividend == NULL || divisor == NULL)
        return NULL;
    assert(txc_int_test_valid(dividend));
    assert(txc_int_test_valid(divisor));
    assert(!txc_int_is_zero(divisor));
    struct txc_int *div = txc_int_create_zero();
    struct txc_int *mod = txc_int_copy(dividend);
    const struct txc_int *const one = txc_int_create_one();
    div->neg = false;
    mod->neg = false;
    while (txc_int_cmp_abs(mod, divisor) >= 0) {
        div = add_acc(div, one);
        if (div == NULL)
            goto clean;
        mod->neg = !divisor->neg;
        mod = add_acc(mod, divisor);
        if (mod == NULL)
            goto clean;
    }
    txc_int_free(one);
    div->neg = dividend->neg ^ divisor->neg;
    mod->neg = false;
    return do_mod ? mod : div;
clean:
    txc_int_free(one);
    txc_int_free(div);
    txc_int_free(mod);
    return NULL;
}

struct txc_int *txc_int_div(const struct txc_int *const dividend, const struct txc_int *const divisor)
{
    return div_mod(dividend, divisor, false);
}

struct txc_int *txc_int_mod(const struct txc_int *const dividend, const struct txc_int *const divisor)
{
    return div_mod(dividend, divisor, true);
}

/* PRINT */

char *txc_int_to_str(const struct txc_int *const integer)
{
    if (integer == NULL)
        return NULL;
    assert(txc_int_test_valid(integer));
    if (integer->used == 0) {
        char *str = txc_strdup("0");
        if (str == NULL)
            TXC_ERROR_ALLOC(strlen("0") + 1, "zero string");
        return str;
    }
    if (integer->used > ((SIZE_MAX - 1 - 3) / 3) / (TXC_INT_ARRAY_TYPE_WIDTH / CHAR_BIT)) {
        TXC_ERROR_OVERFLOW("integer decimal string buffer");
        return NULL;
    }
    const size_t max_len = TXC_INT_ARRAY_TYPE_WIDTH / CHAR_BIT * 3 * integer->used + 1 + 3;
    char *str = malloc(max_len);
    if (str == NULL) {
        TXC_ERROR_ALLOC(max_len, "integer decimal string");
        return NULL;
    }
    size_t int_len = 0;
    for (size_t array_i = integer->used; array_i > 0; array_i--) {
        for (size_t bit_i = TXC_INT_ARRAY_TYPE_WIDTH; bit_i > 0; bit_i--) {
            uint_fast8_t carry = (integer->data[array_i - 1] >> (bit_i - 1)) & 1;
            for (size_t i = 0; i < int_len; i++) {
                if (str[i] >= 5)
                    str[i] += 3;
                str[i] = (str[i] << 1) + carry;
                carry = str[i] > 15 ? 1 : 0;
                str[i] = str[i] & 15;
            }
            if (carry != 0) {
                str[int_len] = 1;
                int_len++;
            }
        }
    }
    const size_t len = integer->neg ? int_len + 3 : int_len;
    char *tmp = realloc(str, len + 1);
    if (tmp != NULL)
        str = tmp;
    str[len] = 0;
    if (integer->neg) {
        str[len - 1] = 0;
        for (size_t i = 0; i < int_len; i++)
            str[int_len - i - 1 + 2] = str[int_len - i - 1] + 48;
        str = txc_strrev(str + 2) - 2;
        str[0] = '(';
        str[1] = '-';
        str[len - 1] = ')';
    } else {
        for (size_t i = 0; i < int_len; i++)
            str[i] += 48;
        str = txc_strrev(str);
    }
    return str;
}
