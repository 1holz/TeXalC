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

#include "common.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "node.h"
#include "integers.h"
#include "util.h"

#define TXC_GROWTH_FACTOR 2

#define TXC_INT_ARRAY_TYPE unsigned char
#define TXC_INT_ARRAY_TYPE_MAX UCHAR_MAX
#define TXC_INT_ARRAY_TYPE_WIDTH CHAR_BIT

#define TXC_ERROR_ALLOC "Could not allocate %zu bytes of memory for %s in %s line %d.\n"
#define TXC_ERROR_NYI "Not yet implemented in %s line %d.\n"
#define TXC_ERROR_OVERFLOW "Overflow was caught for %s in %s line %d.\n"

/* DEFINITIONS */

struct txc_int
{
    TXC_INT_ARRAY_TYPE *data;
    size_t size;
    size_t used;
    bool neg;
};

/* ASSERTS */

static void int_assert_valid(const struct txc_int *const integer)
{
    assert(integer != NULL);
    assert(integer->used <= integer->size);
    if (integer->size > 0)
        assert(integer->data != NULL);
    if (integer->used > 0)
        assert(integer->data[integer->used - 1] > 0);
}

/* MEMORY */

static struct txc_int *int_init(size_t size)
{
    struct txc_int *integer = malloc(sizeof *integer);
    if (integer == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *integer, "int", __FILE__, __LINE__);
        return NULL;
    }
    integer->size = size;
    integer->used = 0;
    integer->neg = false;
    if (size == 0)
        return integer;
    integer->data = malloc(sizeof *(integer->data) * size);
    if (integer->data == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *(integer->data) * size, "initial data", __FILE__, __LINE__);
        free(integer);
        return NULL;
    }
    return integer;
}

static size_t int_inc_size(struct txc_int *const integer, size_t new_size)
{
    int_assert_valid(integer);
    if (integer->used >= SIZE_MAX)
        return 0;
    const size_t max_size = SIZE_MAX / sizeof *(integer->data);
    if (new_size >= max_size)
        new_size = max_size;
    if (new_size <= integer->size)
        return integer->size - integer->used;
    if (integer->size == 0)
        integer->data = NULL;
    TXC_INT_ARRAY_TYPE *tmp = realloc(integer->data, sizeof *(integer->data) * new_size);
    if (tmp == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *(integer->data) * new_size, "new size", __FILE__, __LINE__);
        return integer->size - integer->used;
    }
    integer->data = tmp;
    integer->size = new_size;
    return integer->size - integer->used;
}

static size_t int_inc(struct txc_int *const integer)
{
    size_t new_size = integer->used * TXC_GROWTH_FACTOR;
    if (new_size <= integer->used)
        new_size++;
    return int_inc_size(integer, new_size);
}

static struct txc_int *int_fit(struct txc_int *const integer)
{
    while (integer->used > 0 && integer->data[integer->used - 1] == 0)
        integer->used--;
    int_assert_valid(integer);
    if (integer->size == 0)
        return integer;
    if (integer->used == 0)
    {
        free(integer->data);
        integer->size = 0;
        integer->neg = false;
        return integer;
    }
    TXC_INT_ARRAY_TYPE *tmp = realloc(integer->data, sizeof *(integer->data) * integer->used);
    if (tmp == NULL)
        return integer;
    integer->data = tmp;
    integer->size = integer->used;
    return integer;
}

static struct txc_int *int_shift_bigger(struct txc_int *const integer)
{
    int_assert_valid(integer);
    if (integer->used >= integer->size && integer->data[integer->used - 1] > TXC_INT_ARRAY_TYPE_MAX / 2)
    {
        size_t inc = int_inc(integer);
        if (inc <= 0)
        {
            txc_int_free(integer);
            return NULL;
        }
        integer->used++;
        integer->data[integer->used + 1] = 1;
    }
    bool carry = 0;
    for (size_t i = 0; i < integer->used; i++)
    {
        bool new_carry = integer->data[i] > TXC_INT_ARRAY_TYPE_MAX / 2;
        integer->data[i] <<= 1;
        if (carry)
            integer->data[i] += 1;
        carry = new_carry;
    }
    if (carry)
    {
        integer->data[integer->used] = 1;
        integer->used++;
    }
    return integer;
}

static struct txc_int *int_shift_bigger_amount(struct txc_int *integer, size_t amount)
{
    int_assert_valid(integer);
    for (size_t i = 0; i < amount; i++)
        integer = int_shift_bigger(integer);
    return integer;
}

static struct txc_int *int_shift_bigger_wide_amount(struct txc_int *const integer, size_t amount)
{
    int_assert_valid(integer);
    size_t free = int_inc_size(integer, integer->used + amount);
    if (free < amount)
    {
        txc_int_free(integer);
        return NULL;
    }
    for (size_t i = 0; i < integer->used; i++)
        integer->data[amount + i] = integer->data[i];
    for (size_t i = 0; i < amount; i++)
        integer->data[i] = 0;
    integer->used = integer->used + amount;
    return integer;
}

static struct txc_int *int_shift_smaller(struct txc_int *const integer)
{
    int_assert_valid(integer);
    if (txc_int_is_zero(integer))
        return integer;
    for (size_t i = 0; i < integer->used - 1; i++)
    {
        integer->data[i] >>= 1;
        if (integer->data[i + 1] % 2 > 0)
            integer->data[i] += TXC_INT_ARRAY_TYPE_MAX / 2 + 1;
    }
    integer->data[integer->used - 1] >>= 1;
    if (integer->data[integer->used - 1] == 0)
    {
        integer->used--;
        if (integer->used <= 0)
        {
            free(integer->data);
            integer->neg = false;
            integer->size = 0;
        }
    }
    return integer;
}

static struct txc_int *int_shift_smaller_amount(struct txc_int *integer, size_t amount)
{
    int_assert_valid(integer);
    for (size_t i = 0; i < amount; i++)
    {
        integer = int_shift_smaller(integer);
        if (integer->used <= 0)
            break;
    }
    return integer;
}

static struct txc_int *int_shift_smaller_wide_amount(struct txc_int *const integer, size_t amount)
{
    int_assert_valid(integer);
    if (amount > integer->used)
        amount = integer->used;
    integer->used = integer->used - amount;
    for (size_t i = 0; i < integer->used; i++)
        integer->data[i] = integer->data[i + amount];
    return integer;
}

static struct txc_int *int_from_bin_str(struct txc_int *integer, const char *const str, const size_t len)
{
    for (size_t i = 0; i < len; i++)
        assert('0' <= str[i] && str[i] <= '1');
    const size_t bin_width = 1;
    const size_t chars_per_elem = TXC_INT_ARRAY_TYPE_WIDTH / bin_width;
    for (size_t i = 0; i < len; i++)
    {
        if (i % chars_per_elem == 0)
        {
            integer->data[integer->used] = 0;
            integer->used++;
        }
        integer->data[integer->used - 1] += (str[len - i - 1] - 48) << ((i % chars_per_elem) * bin_width);
    }
    return integer;
}

static struct txc_int *int_from_dec_str(struct txc_int *integer, const char *const str, const size_t len)
{
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
    while (buf_len > 0)
    {
        if (buf[buf_len - 1] == 0)
        {
            buf_len--;
            continue;
        }
        bool carry = false;
        for (size_t ii = 0; ii < buf_len; ii++)
        {
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
        if (bit_i >= TXC_INT_ARRAY_TYPE_WIDTH - 1)
        {
            integer->data[integer->used] = 0;
            integer->used++;
            bit_i = 0;
        }
        else
            bit_i++;
    }
    return integer;
}

static struct txc_int *int_from_hex_str(struct txc_int *integer, const char *const str, const size_t len)
{
    for (size_t i = 0; i < len; i++)
        assert(('0' <= str[i] && str[i] <= '9') || ('A' <= str[i] && str[i] <= 'F') || ('a' <= str[i] && str[i] <= 'f'));
    const size_t hex_width = 4;
    const size_t chars_per_elem = TXC_INT_ARRAY_TYPE_WIDTH / hex_width;
    for (size_t i = 0; i < len; i++)
    {
        if (i % chars_per_elem == 0)
        {
            integer->data[integer->used] = 0;
            integer->used++;
        }
        const char buf[2] = {str[len - i - 1], 0};
        integer->data[integer->used - 1] += strtoul((const char *restrict)&buf, NULL, 16) << ((i % chars_per_elem) * hex_width);
    }
    return integer;
}

static struct txc_int *bake(struct txc_int *const integer)
{
    return integer;
}

txc_node *txc_int_create_int(const char *const str, size_t len, uint_fast8_t base)
{
    if (base != 2 && base != 10 && base != 16)
    {
        fprintf(stderr, TXC_ERROR_NYI, __FILE__, __LINE__);
        return (txc_node *)&TXC_NAN_ERROR_NYI;
    }
    const char *cur = str;
    size_t width = 4;
    if (base == 2)
        width = 1;
    for (; cur[0] == '0'; len--)
        cur++;
    if (len <= 0)
        return txc_int_to_node(txc_int_create_zero());
    size_t chars_per_elem = TXC_INT_ARRAY_TYPE_WIDTH / width;
    txc_int *integer = int_init(len / chars_per_elem + 1);
    if (integer == NULL)
        return NULL;
    switch (base)
    {
    case 2:
        integer = int_from_bin_str(integer, cur, len);
        break;
    default: /* FALLTHROUGH */
    case 10:
        integer = int_from_dec_str(integer, cur, len);
        break;
    case 16:
        integer = int_from_hex_str(integer, cur, len);
        break;
    }
    return txc_int_to_node(bake(int_fit(integer)));
}

txc_int *txc_int_create_zero(void)
{
    struct txc_int *zero = int_init(0);
    if (zero == NULL)
        return NULL;
    return bake(zero);
}

txc_int *txc_int_create_one(void)
{
    struct txc_int *one = int_init(1);
    if (one == NULL)
        return NULL;
    one->used = 1;
    one->data[0] = 1;
    return bake(one);
}

txc_int *txc_int_copy_read(const txc_int *const from)
{
    return txc_int_copy_write(from);
}

txc_int *txc_int_copy_write(const txc_int *const from)
{
    int_assert_valid(from);
    txc_int *copy = int_init(from->used);
    if (copy == NULL)
        return NULL;
    copy->used = from->used;
    copy->size = from->used;
    copy->neg = from->neg;
    if (copy->size == 0)
        return copy;
    for (size_t i = 0; i < copy->used; i++)
        copy->data[i] = from->data[i];
    return copy;
}

void txc_int_free(txc_int *const integer)
{
    int_assert_valid(integer);
    if (integer->size > 0)
        free(integer->data);
    free(integer);
}

/* CONVERT */

txc_node *txc_int_to_node(txc_int *const integer)
{
    int_assert_valid(integer);
    union impl impl;
    impl.integer = integer;
    return txc_node_create(NULL, impl, 0, TXC_INT);
}

/* INT */

bool txc_int_is_pos_one(const txc_int *test)
{
    int_assert_valid(test);
    return test->used == 1 && test->neg == false && test->data[0] == 1;
}

bool txc_int_is_zero(const txc_int *test)
{
    int_assert_valid(test);
    return test->used == 0;
}

bool txc_int_is_neg_one(const txc_int *test)
{
    int_assert_valid(test);
    return test->used == 1 && test->neg == true && test->data[0] == 1;
}

bool txc_int_is_neg(const txc_int *test)
{
    int_assert_valid(test);
    return !txc_int_is_zero(test) && test->neg == true;
}

int_fast8_t txc_int_cmp_abs(const txc_int *const a, const txc_int *const b)
{
    int_assert_valid(a);
    int_assert_valid(b);
    if (a->used == 0 && b->used == 0)
        return 0;
    if (a->used < b->used)
        return -1;
    if (a->used > b->used)
        return 1;
    size_t used = a->used;
    for (size_t i = 0; i < used; i++)
    {
        if (a->data[used - i - 1] < b->data[used - i - 1])
            return -1;
        if (a->data[used - i - 1] > b->data[used - i - 1])
            return 1;
    }
    return 0;
}

int_fast8_t txc_int_cmp(const txc_int *const a, const txc_int *const b)
{
    int_assert_valid(a);
    int_assert_valid(b);
    if (a->used == 0 && b->used == 0)
        return 0;
    if (a->neg && !b->neg)
        return 1;
    if (!a->neg && b->neg)
        return -1;
    return txc_int_cmp_abs(a, b) * (a->neg && b->neg ? -1 : 1);
}

txc_int *txc_int_neg(const txc_int *const integer)
{
    int_assert_valid(integer);
    struct txc_int *result = txc_int_copy_write(integer);
    if (result == NULL)
        return NULL;
    result->neg = !result->neg;
    return bake(result);
}

static struct txc_int *int_add_acc(struct txc_int *acc, const struct txc_int *const summand)
{
    int_assert_valid(acc);
    int_assert_valid(summand);
        bool carry = false;
    if (acc->neg != summand->neg)
        {
        int_fast8_t abs_cmp = txc_int_cmp_abs(acc, summand);
            if (abs_cmp == 0)
            {
                txc_int_free(acc);
            return txc_int_create_zero();
            }
            struct txc_int *smaller = acc;
            if (abs_cmp < 0)
            acc = txc_int_copy_write(summand);
            else
            smaller = txc_int_copy_write(summand);
            if (acc == NULL || smaller == NULL)
            {
                txc_int_free(acc);
                return NULL;
            }
        for (size_t i = 0; i < acc->used; i++)
            {
            if (smaller->used <= i && !carry)
                    break;
                if (carry)
                {
                carry = acc->data[i] <= 0;
                acc->data[i]--;
                }
            if (i < smaller->used)
                {
                if (acc->data[i] < smaller->data[i])
                        carry = true;
                acc->data[i] -= smaller->data[i];
            }
        }
            txc_int_free(smaller);
        while (acc->data[acc->used - 1] == 0)
            acc->used--;
        }
        else
        {
        size_t result_size = summand->used > acc->used ? summand->used : acc->used;
            int_inc_size(acc, result_size);
            if (acc->size < result_size)
            {
                txc_int_free(acc);
                return NULL;
            }
        for (size_t i = 0; i < result_size; i++)
            {
            if (i >= acc->used)
                {
                    acc->data[acc->used] = 0;
                    acc->used++;
                }
            if (summand->used <= i && !carry)
                    break;
                if (carry)
                {
                carry = acc->data[i] >= TXC_INT_ARRAY_TYPE_MAX;
                acc->data[i]++;
                }
            if (i < summand->used)
                {
                if (TXC_INT_ARRAY_TYPE_MAX - acc->data[i] < summand->data[i])
                        carry = true;
                acc->data[i] += summand->data[i];
                }
            }
            if (carry)
            {
                if (acc->used >= acc->size && int_inc(acc) <= 0)
                {
                    txc_int_free(acc);
                    return NULL;
                }
                acc->data[acc->used] = 1;
                acc->used++;
            }
        }
    return acc;
}

txc_int *txc_int_add(const txc_int *const *const summands, const size_t len)
{
    if (len > 0)
    {
        assert(summands != NULL);
        for (size_t i = 0; i < len; i++)
            int_assert_valid(summands[i]);
    }
    else
    {
        return txc_int_create_zero();
    }
    struct txc_int *acc = txc_int_create_zero();
    if (acc == NULL)
        return NULL;
    for (size_t i = 0; i < len; i++)
    {
        if (summands[i]->used > 0)
            acc = int_add_acc(acc, summands[i]);
    }
    if (acc != NULL)
        int_fit(acc);
    return bake(acc);
}

txc_int *txc_int_mul(const txc_int *const *const factors, const size_t len)
{
    if (len > 0)
    {
        assert(factors != NULL);
        for (size_t i = 0; i < len; i++)
            int_assert_valid(factors[i]);
    }
    else
    {
        return txc_int_create_zero();
    }
    for (size_t i = 0; i < len; i++)
    {
        if (factors[i]->used == 0)
            return txc_int_create_zero();
        }
    bool neg = factors[0]->neg;
    txc_int *acc = txc_int_copy_write(factors[0]);
    if (acc == NULL)
        return NULL;
    for (size_t i = 1; i < len; i++)
    {
        if (factors[i]->neg)
            neg = !neg;
        size_t result_size = acc->used + factors[i]->used;
        int_inc_size(acc, result_size);
        if (acc->size < result_size)
        {
            txc_int_free(acc);
            return NULL;
        }
        size_t summand_i = 0;
        for (size_t j = 0; j < factors[i]->used; j++)
            for (size_t k = 0; k < TXC_INT_ARRAY_TYPE_WIDTH; k++)
                if (((factors[i]->data[j] >> k) & 1) != 0)
                summand_i++;
        txc_int *summands[summand_i];
        summand_i = 0;
        for (size_t j = 0; j < factors[i]->used; j++)
        {
            bool failed = false;
            for (size_t k = 0; k < TXC_INT_ARRAY_TYPE_WIDTH; k++)
            {
                if (((factors[i]->data[j] >> k) & 1) == 0)
                    continue;
                summands[summand_i] = txc_int_copy_write(acc);
                if (summands[summand_i] == NULL)
                {
                    failed = true;
                    break;
                }
                summands[summand_i] = int_shift_bigger_wide_amount(summands[summand_i], j);
                if (summands[summand_i] == NULL)
                {
                    failed = true;
                    break;
                }
                summands[summand_i] = int_shift_bigger_amount(summands[summand_i], k);
                if (summands[summand_i] == NULL)
                {
                    failed = true;
                    break;
                }
                summand_i++;
            }
            if (failed)
            {
                for (size_t l = 0; l < summand_i; l++)
                    txc_int_free(summands[l]);
                txc_int_free(acc);
                return NULL;
            }
        }
        txc_int_free(acc);
        acc = txc_int_add((const txc_int **)summands, summand_i);
    }
    if (acc != NULL)
        int_fit(acc);
    acc->neg = neg;
    bake(acc);
    return acc;
}

/* PRINT */

const char *txc_int_to_str(txc_int *const integer)
{
    int_assert_valid(integer);
    if (integer->used == 0)
    {
        char *str = malloc(2);
        if (str == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, (size_t)2, "zero string", __FILE__, __LINE__);
            return NULL;
        }
        str[0] = '0';
        str[1] = 0;
        return str;
    }
    if (integer->used > ((SIZE_MAX - 1 - 3) / 3) / (TXC_INT_ARRAY_TYPE_WIDTH / CHAR_BIT))
    {
        fprintf(stderr, TXC_ERROR_OVERFLOW, "integer decimal string buffer", __FILE__, __LINE__);
        return NULL;
    }
    const size_t max_len = TXC_INT_ARRAY_TYPE_WIDTH / CHAR_BIT * 3 * integer->used + 1 + 3;
    char *str = malloc(max_len);
    if (str == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, max_len, "integer decimal string", __FILE__, __LINE__);
        return NULL;
    }
    size_t int_len = 0;
    for (size_t array_i = integer->used; array_i > 0; array_i--)
    {
        for (size_t bit_i = TXC_INT_ARRAY_TYPE_WIDTH; bit_i > 0; bit_i--)
        {
            uint_fast8_t carry = (integer->data[array_i - 1] >> (bit_i - 1)) & 1;
            for (size_t i = 0; i < int_len; i++)
            {
                if (str[i] >= 5)
                    str[i] += 3;
                str[i] = (str[i] << 1) + carry;
                carry = str[i] > 15 ? 1 : 0;
                str[i] = str[i] & 15;
            }
            if (carry != 0)
            {
                str[int_len] = 1;
                int_len++;
            }
        }
    }
    size_t len = integer->neg ? int_len + 3 : int_len;
    char *tmp = realloc(str, len + 1);
    if (tmp != NULL)
        str = tmp;
    str[len] = 0;
    if (integer->neg)
    {
        str[len - 1] = 0;
        for (size_t i = 0; i < int_len; i++)
            str[int_len - i - 1 + 2] = str[int_len - i - 1] + 48;
        str = txc_strrev(str + 2) - 2;
        str[0] = '(';
        str[1] = '-';
        str[len - 1] = ')';
    }
    else
    {
        for (size_t i = 0; i < int_len; i++)
            str[i] += 48;
        str = txc_strrev(str);
    }
    return str;
}
