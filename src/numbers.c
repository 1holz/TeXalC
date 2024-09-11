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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "node.h"
#include "numbers.h"
#include "util.h"

#define TXC_GROWTH_FACTOR 2

#define TXC_NUM_ARRAY_TYPE unsigned char
#define TXC_NUM_ARRAY_TYPE_MAX UCHAR_MAX
#define TXC_NUM_ARRAY_TYPE_WIDTH CHAR_BIT

#define TXC_ERROR_NYI "Not yet implemented in %s line %d.\n"
#define TXC_ERROR_ILLEGAL_ARG "Illegal argument %s in %s line %d.\n"
#define TXC_ERROR_ALLOC "Could not allocate %zu bytes of memory for %s in %s line %d.\n"
#define TXC_ERROR_INVALID_NUM_TYPE "Number type %du is invalid in %s line %d.\n"

#define TXC_PRINT_ERROR_NYI "\\text{PRINT(not yet implemented)}"
#define TXC_PRINT_ERROR_INVALID_NUM_TYPE "\\text{PRINT(Number type is invalid. Please see stderr for more information.)}"

/* DEFINITIONS */

struct txc_num_array
{
    TXC_NUM_ARRAY_TYPE *data;
    size_t size;
    size_t used;
};

/* ASSERTS */

static void num_array_assert_valid(const struct txc_num_array *const array)
{
    assert(array != NULL);
    assert(array->size == 0 || array->data != NULL);
    assert(array->used <= array->size);
}

/* NUM ARRAY */

static struct txc_num_array *num_array_init(size_t size)
{
    struct txc_num_array *array = malloc(sizeof *array);
    if (array == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *array, "array", __FILE__, __LINE__);
        return NULL;
    }
    array->size = size;
    array->used = 0;
    if (size == 0)
        return array;
    array->data = malloc(sizeof *(array->data) * size);
    if (array->data == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *(array->data) * size, "initial data", __FILE__, __LINE__);
        free(array);
        return NULL;
    }
    return array;
}

static size_t num_array_inc_amount(struct txc_num_array *const array, size_t new_size)
{
    num_array_assert_valid(array);
    const size_t max_size = SIZE_MAX / sizeof *(array->data);
    if (array->used >= SIZE_MAX)
        return 0;
    if (new_size <= array->used)
        new_size = array->used + 1;
    else if (array->used >= max_size / TXC_GROWTH_FACTOR)
        new_size = max_size;
    if (new_size <= array->size)
        return array->size - array->used;
    if (array->size == 0)
        array->data = NULL;
    TXC_NUM_ARRAY_TYPE *tmp = realloc(array->data, sizeof *(array->data) * new_size);
    if (tmp == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *(array->data) * new_size, "new size", __FILE__, __LINE__);
        return array->size - array->used;
    }
    array->data = tmp;
    array->size = new_size;
    return array->size - array->used;
}

static size_t num_array_inc(struct txc_num_array *const array)
{
    return num_array_inc_amount(array, array->used * TXC_GROWTH_FACTOR);
}

static struct txc_num_array *num_array_fit(struct txc_num_array *const array)
{
    num_array_assert_valid(array);
    if (array->size == 0)
        return array;
    while (array->used > 0 && array->data[array->used - 1] == 0)
        array->used--;
    if (array->used == 0)
    {
        free(array->data);
        array->size = 0;
        return array;
    }
    TXC_NUM_ARRAY_TYPE *tmp = realloc(array->data, sizeof *(array->data) * array->used);
    if (tmp == NULL)
        return array;
    array->data = tmp;
    array->size = array->used;
    return array;
}

static struct txc_num_array *num_array_shift_bigger(struct txc_num_array *const array)
{
    num_array_assert_valid(array);
    if (array->used >= array->size && array->data[array->used - 1] > TXC_NUM_ARRAY_TYPE_MAX / 2)
    {
        if (num_array_inc(array) <= 0)
            return NULL;
        array->used++;
        array->data[array->used + 1] = 1;
    }
    bool carry = 0;
    for (size_t i = 0; i < array->used; i++)
    {
        bool new_carry = array->data[i] > TXC_NUM_ARRAY_TYPE_MAX / 2;
        array->data[i] <<= 1;
        if (carry)
            array->data[i] += 1;
        carry = new_carry;
    }
    if (carry)
    {
        array->data[array->used] = 1;
        array->used++;
    }
    return array;
}

static struct txc_num_array *num_array_shift_smaller(struct txc_num_array *const array)
{
    num_array_assert_valid(array);
    if (array->used == 0)
        return array;
    size_t cur = 0;
    while (cur < array->used - 1)
    {
        array->data[cur] >>= 1;
        if (array->data[cur] % 2)
            array->data[cur] += TXC_NUM_ARRAY_TYPE_MAX / 2 + 1;
        cur++;
    }
    array->data[cur] >>= 1;
    return array;
}

static struct txc_num_array *num_array_from_bin_str(struct txc_num_array *array, const char *const str, const size_t len)
{
    for (size_t i = 0; i < len; i++)
        assert('0' <= str[i] && str[i] <= '1');
    const size_t bin_width = 1;
    const size_t chars_per_elem = TXC_NUM_ARRAY_TYPE_WIDTH / bin_width;
    for (size_t i = 0; i < len; i++)
    {
        if (i % chars_per_elem == 0)
        {
            array->data[array->used] = 0;
            array->used++;
        }
        array->data[array->used - 1] += (str[len - i - 1] - 48) << ((i % chars_per_elem) * bin_width);
    }
    return array;
}

static struct txc_num_array *num_array_from_dec_str(struct txc_num_array *array, const char *const str, const size_t len)
{
    for (size_t i = 0; i < len; i++)
        assert('0' <= str[i] && str[i] <= '9');
    const size_t dec_width = 4;
    unsigned char buf[len];
    for (size_t i = 0; i < len; i++)
        buf[i] = str[len - i - 1] - 48;
    array->data[0] = 0;
    array->used = 1;
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
            array->data[array->used - 1] += 1 << bit_i;
        if (bit_i >= TXC_NUM_ARRAY_TYPE_WIDTH - 1)
        {
            array->data[array->used] = 0;
            array->used++;
            bit_i = 0;
        }
        else
            bit_i++;
    }
    return array;
}

static struct txc_num_array *num_array_from_hex_str(struct txc_num_array *array, const char *const str, const size_t len)
{
    for (size_t i = 0; i < len; i++)
        assert(('0' <= str[i] && str[i] <= '9') || ('A' <= str[i] && str[i] <= 'F') || ('a' <= str[i] && str[i] <= 'f'));
    const size_t hex_width = 4;
    const size_t chars_per_elem = TXC_NUM_ARRAY_TYPE_WIDTH / hex_width;
    for (size_t i = 0; i < len; i++)
    {
        if (i % chars_per_elem == 0)
        {
            array->data[array->used] = 0;
            array->used++;
        }
        const char buf[2] = {str[len - i - 1], 0};
        array->data[array->used - 1] += strtoul((const char *restrict)&buf, NULL, 16) << ((i % chars_per_elem) * hex_width);
    }
    return array;
}

/* CREATE, COPY AND FREE */

txc_node *txc_create_natural_num_or_zero(const char *const str, size_t len)
{
    uint_fast8_t base = 10;
    const char *cur = str;
    size_t width = 4;
    if (len >= 2)
    {
        if (!strncasecmp(str, "0b", 2))
        {
            len -= 2;
            base = 2;
            cur += 2;
            width = 1;
        }
        else if (!strncasecmp(str, "0x", 2))
        {
            len -= 2;
            base = 16;
            cur += 2;
            width = 4;
        }
    }
    for (; cur[0] == '0'; len--)
        cur++;
    size_t chars_per_elem = TXC_NUM_ARRAY_TYPE_WIDTH / width;
    union impl impl;
    impl.natural_num = num_array_init(len / chars_per_elem + 1);
    if (len <= 0)
        return txc_node_create(NULL, impl, 0, TXC_NUM);
    if (impl.natural_num == NULL)
        return NULL;
    switch (base)
    {
    case 2:
        impl.natural_num = num_array_from_bin_str(impl.natural_num, cur, len);
        break;
    default: /* FALLTHROUGH */
    case 10:
        impl.natural_num = num_array_from_dec_str(impl.natural_num, cur, len);
        break;
    case 16:
        impl.natural_num = num_array_from_hex_str(impl.natural_num, cur, len);
        break;
    }
    impl.natural_num = num_array_fit(impl.natural_num);
    return txc_node_create(NULL, impl, 0, TXC_NUM);
}

txc_num *txc_copy_num(txc_num *const from)
{
    num_array_assert_valid(from);
    txc_num *copy = num_array_init(from->used);
    if (copy == NULL)
        return NULL;
    copy->used = from->used;
    copy->size = from->used;
    if (copy->size == 0)
        return copy;
    for (size_t i = 0; i < copy->used; i++)
        copy->data[i] = from->data[i];
    return copy;
}

void txc_free_num(txc_num *const num)
{
    num_array_assert_valid(num);
    if (num->size > 0)
        free(num->data);
    free(num);
}

/* NUM */

/*
txc_num *txc_num_add(txc_num *const *const summands, const size_t len)
{
    // TODO assert valid
    txc_num *acc = NULL;
    bool finish = false;
    for (size_t i = 0; i < len; i++)
    {
        switch (summands[i]->type)
        {
        case TXC_NAN:
            acc = txc_copy_num(summands[i]);
            finish = true;
            break;
        case TXC_NATURAL_NUM:
            if (acc == NULL)
            {
                acc = txc_copy_num(summands[i]);
                break;
            }
            struct txc_num_array *array = acc->impl.natural_num;
            size_t larger_size = array->used;
            if (summands[i]->impl.natural_num->used > array->used)
                larger_size = summands[i]->impl.natural_num->used;
            num_array_inc_amount(array, larger_size);
            if (array->size < larger_size)
            {
                acc = (txc_num *)&TXC_NAN_ERROR_ALLOC;
                finish = true;
                break;
            }
            bool carry = false;
            for (size_t j = 0; j < larger_size; j++)
            {
                if (j >= array->used)
                {
                    array->data[array->used] = 0;
                    array->used++;
                }
                if (summands[i]->impl.natural_num->used <= j && !carry)
                    break;
                if (carry)
                {
                    if (array->data[j] >= TXC_NUM_ARRAY_TYPE_MAX)
                    {
                        array->data[j] = 0;
                        carry = true;
                    }
                    else
                    {
                        array->data[j]++;
                        carry = false;
                    }
                }
                if (j < summands[i]->impl.natural_num->used)
                {
                    TXC_NUM_ARRAY_TYPE to_add = summands[i]->impl.natural_num->data[j];
                    if (TXC_NUM_ARRAY_TYPE_MAX - array->data[j] < to_add)
                        carry = true;
                    array->data[j] += to_add;
                }
            }
            if (carry)
            {
                if (array->used >= array->size && num_array_inc(array) <= 0)
                {
                    acc = (txc_num *)&TXC_NAN_ERROR_ALLOC;
                    finish = true;
                    break;
                }
                array->data[array->used] = 1;
                array->used++;
            }
            break;
        default:
            fprintf(stderr, TXC_ERROR_INVALID_NUM_TYPE, summands[i]->type, __FILE__, __LINE__);
            acc = (txc_num *)&TXC_NAN_ERROR_INVALID_NUM_TYPE;
            finish = true;
            break;
        }
        if (finish)
            break;
    }
    for (size_t i = 0; i < len; i++)
        txc_free_num(summands[i]);
    if (acc == NULL)
        acc = (txc_num *)&TXC_ZERO_ZERO;
    if (acc->type == TXC_NATURAL_NUM)
        num_array_fit(acc->impl.natural_num);
    return acc;
}
*/

const char *txc_num_to_str(txc_num *const num)
{
    num_array_assert_valid(num);
    const size_t max_len = TXC_NUM_ARRAY_TYPE_WIDTH / CHAR_BIT * 3 * num->used + 1;
    char *str = malloc(max_len);
    if (str == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, max_len, "natural number decimal string", __FILE__, __LINE__);
        return NULL;
    }
    size_t len = 0;
    for (size_t array_i = num->used; array_i > 0; array_i--)
    {
        for (size_t bit_i = TXC_NUM_ARRAY_TYPE_WIDTH; bit_i > 0; bit_i--)
        {
            uint_fast8_t carry = (num->data[array_i - 1] >> (bit_i - 1)) & 1;
            for (size_t i = 0; i < len; i++)
            {
                if (str[i] >= 5)
                    str[i] += 3;
                str[i] = (str[i] << 1) + carry;
                carry = str[i] > 15 ? 1 : 0;
                str[i] = str[i] & 15;
            }
            if (carry != 0)
            {
                str[len] = 1;
                len++;
            }
        }
    }
    char *tmp = realloc(str, len + 1);
    if (tmp != NULL)
        str = tmp;
    str[len] = 0;
    for (size_t i = 0; i < len; i++)
        str[i] += 48;
    str = txc_strrev(str);
    return str;
}
