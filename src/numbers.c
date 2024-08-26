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

#include "numbers.h"
#include "util.h"

#define TXC_GROWTH_FACTOR 2

#define TXC_NUM_ARRAY_TYPE unsigned char
#define TXC_NUM_ARRAY_TYPE_MAX UCHAR_MAX
#define TXC_NUM_ARRAY_TYPE_WIDTH CHAR_BIT

#define TXC_ERROR_NYI "Not yet implemented in %s line %d.\n"
#define TXC_ERROR_ILLEGAL_ARG "Illegal argument %s in %s line %d.\n"
#define TXC_ERROR_ALLOC "Could not allocate %zu bytes of memory for %s in %s line %d.\n"
#define TXC_ERROR_UNKNOWN_NUM_TYPE "Number type %du is unknownin %s line %d.\n"

#define TXC_PRINT_NAN "\\text{NAN(%s)}"
#define TXC_PRINT_NAN_ERROR_ALLOC "\\text{NAN(Could not allocate enough memory. Please see stderr for more information.)}"
#define TXC_PRINT_NAN_ERROR_NYI "\\text{NAN(Not yet implemented. Please see stderr for more information.)}"
#define TXC_PRINT_NAN_UNSPECIFIED "\\text{NAN(unspecified)}"

#define TXC_PRINT_NIL "\\text{NIL(%s)}"
#define TXC_PRINT_NIL_UNSPECIFIED "\\text{NIL(unspecified)}"

#define TXC_PRINT_ZERO "0"

#define TXC_PRINT_ERROR_NYI "\\text{PRINT(not yet implemented)}"
#define TXC_PRINT_ERROR_UNKNOWN_NUM_TYPE "\\text{PRINT(Number type is unknown. Please see stderr for more information.)}"

struct txc_num_array
{
    TXC_NUM_ARRAY_TYPE *data;
    size_t size;
    size_t used;
};

enum txc_num_type
{
    TXC_NAN,
    TXC_NIL,
    TXC_ZERO,
    TXC_NATURAL_NUM
};

struct txc_num
{
    union
    {
        struct txc_num_array *natural_num;
    } impl;
    const char *str;
    enum txc_num_type type;
    bool singleton;
};

/* SINGLETONS */

const txc_num TXC_NAN_ERROR_ALLOC = {.str = TXC_PRINT_NAN_ERROR_ALLOC,
                                     .type = TXC_NAN,
                                     .singleton = true};
const txc_num TXC_NAN_ERROR_NYI = {.str = TXC_PRINT_NAN_ERROR_NYI,
                                   .type = TXC_NAN,
                                   .singleton = true};
const txc_num TXC_NAN_UNSPECIFIED = {.str = TXC_PRINT_NAN_UNSPECIFIED,
                                     .type = TXC_NAN,
                                     .singleton = true};

const txc_num TXC_NIL_UNSPECIFIED = {.str = TXC_PRINT_NIL_UNSPECIFIED,
                                     .type = TXC_NIL,
                                     .singleton = true};

const txc_num TXC_ZERO_ZERO = {.str = TXC_PRINT_ZERO,
                               .type = TXC_ZERO,
                               .singleton = true};

/* NUM ARRAY */

static void txc_num_array_assert_valid(const struct txc_num_array *const array)
{
    assert(array != NULL);
    assert(array->data != NULL);
    assert(array->used <= array->size);
}

static struct txc_num_array *txc_num_array_init(size_t size)
{
    if (size <= 0)
    {
        fprintf(stderr, TXC_ERROR_ILLEGAL_ARG, "size", __FILE__, __LINE__);
        return NULL;
    }
    struct txc_num_array *array = malloc(sizeof *array);
    if (array == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *array, "array", __FILE__, __LINE__);
        return NULL;
    }
    array->data = malloc(sizeof *(array->data) * size);
    if (array->data == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *(array->data) * size, "initial data", __FILE__, __LINE__);
        free(array);
        return NULL;
    }
    array->size = size;
    array->used = 0;
    return array;
}

static size_t txc_num_array_inc(struct txc_num_array *const array)
{
    txc_num_array_assert_valid(array);
    if (array->used >= SIZE_MAX)
        return 0;
    size_t new_size = SIZE_MAX / sizeof *(array->data) / (TXC_NUM_ARRAY_TYPE_WIDTH / CHAR_BIT);
    if (array->used >= new_size / TXC_GROWTH_FACTOR)
    {
        TXC_NUM_ARRAY_TYPE *tmp = realloc(array->data, sizeof *(array->data) * new_size);
        if (tmp == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, sizeof *(array->data) * new_size, "max size", __FILE__, __LINE__);
            return array->size - array->used;
        }
        array->data = tmp;
        array->size = SIZE_MAX;
        return array->size - array->used;
    }
    new_size = array->used * TXC_GROWTH_FACTOR;
    if (new_size <= array->used)
        new_size = array->used + 1;
    if (new_size <= array->size)
        return array->size - array->used;
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

static size_t txc_num_array_fit(struct txc_num_array *const array)
{
    txc_num_array_assert_valid(array);
    while (array->used > 0 && array->data[array->used - 1] == 0)
        (array->used)--;
    size_t new_size = array->used <= 0 ? 1 : array->used;
    TXC_NUM_ARRAY_TYPE *tmp = realloc(array->data, sizeof *(array->data) * new_size);
    if (tmp == NULL)
        return array->size - array->used;
    array->data = tmp;
    array->size = new_size;
    return array->size - array->used;
}

static struct txc_num_array *txc_num_array_shift_bigger(struct txc_num_array *const array)
{
    txc_num_array_assert_valid(array);
    if (array->used >= array->size && array->data[array->used - 1] > TXC_NUM_ARRAY_TYPE_MAX / 2)
    {
        if (txc_num_array_inc(array) <= 0)
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
        (array->used)++;
    }
    return array;
}

static struct txc_num_array *txc_num_array_shift_smaller(struct txc_num_array *const array)
{
    txc_num_array_assert_valid(array);
    if (!array->used)
        return array;
    size_t cur = 0;
    while (cur < array->used - 1)
    {
        array->data[cur] >>= 1;
        if (array->data[cur] % 2)
            array->data[cur] += TXC_NUM_ARRAY_TYPE_MAX / 2 + 1;
    }
    array->data[cur] >>= 1;
    return array;
}

static struct txc_num_array *txc_num_array_from_bin_str(struct txc_num_array *array, const char *const str, const size_t len)
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
            (array->used)++;
        }
        array->data[array->used - 1] += (str[len - i - 1] - 48) << ((i % chars_per_elem) * bin_width);
    }
    return array;
}

static struct txc_num_array *txc_num_array_from_dec_str(struct txc_num_array *array, const char *const str, const size_t len)
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
            (array->used)++;
            bit_i = 0;
        }
        else
            bit_i++;
    }
    return array;
}

static struct txc_num_array *txc_num_array_from_hex_str(struct txc_num_array *array, const char *const str, const size_t len)
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
            (array->used)++;
        }
        const char buf[2] = {str[len - i - 1], 0};
        array->data[array->used - 1] += strtoul((const char *restrict)&buf, NULL, 16) << ((i % chars_per_elem) * hex_width);
    }
    return array;
}

/* CREATE */

txc_num *txc_create_nan(const char *const reason)
{
    if (reason == NULL)
        return (txc_num *)&TXC_NAN_UNSPECIFIED;
    txc_num *nan = malloc(sizeof *nan);
    if (nan == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *nan, "nan", __FILE__, __LINE__);
        return (txc_num *)&TXC_NAN_ERROR_ALLOC;
    }
    nan->str = txc_strdup(reason);
    if (nan->str == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *nan, "nan reason", __FILE__, __LINE__);
        free(nan);
        return (txc_num *)&TXC_NAN_ERROR_ALLOC;
    }
    nan->type = TXC_NAN;
    nan->singleton = false;
    return nan;
}

txc_num *txc_create_nil(const char *const reason)
{
    if (reason == NULL)
        return (txc_num *)&TXC_NIL_UNSPECIFIED;
    txc_num *nil = malloc(sizeof *nil);
    if (nil == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *nil, "nil", __FILE__, __LINE__);
        return (txc_num *)&TXC_NAN_ERROR_ALLOC;
    }
    nil->str = txc_strdup(reason);
    if (nil->str == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *nil, "nil reason", __FILE__, __LINE__);
        free(nil);
        return (txc_num *)&TXC_NAN_ERROR_ALLOC;
    }
    nil->type = TXC_NIL;
    nil->singleton = false;
    return nil;
}

txc_num *txc_create_natural_num_or_zero(const char *const str, size_t len)
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
        }
        else if (!strncasecmp(str, "0x", 2))
        {
            len -= 2;
            base = 16;
            cur += 2;
            width = 1;
        }
    }
    for (; cur[0] == '0'; len--)
        cur++;
    if (len <= 0)
        return (txc_num *)&TXC_ZERO_ZERO;
size_t chars_per_elem = TXC_NUM_ARRAY_TYPE_WIDTH / width;
    struct txc_num_array *array = txc_num_array_init(len / chars_per_elem + 1);
    if (array == NULL)
        return (txc_num *)&TXC_NAN_ERROR_ALLOC;
    switch (base)
    {
    case 2:
        array = txc_num_array_from_bin_str(array, cur, len);
        break;
    default: /* FALLTHROUGH */
    case 10:
        array = txc_num_array_from_dec_str(array, cur, len);
        break;
    case 16:
        array = txc_num_array_from_hex_str(array, cur, len);
        break;
    }
    if (array == NULL)
        return (txc_num *)&TXC_NAN_ERROR_ALLOC;
    txc_num_array_fit(array);
    txc_num *num = malloc(sizeof *num);
    if (num == NULL)
    {
        fprintf(stderr, TXC_ERROR_ALLOC, sizeof *num, "number", __FILE__, __LINE__);
        free(array);
        return (txc_num *)&TXC_NAN_ERROR_ALLOC;
    }
    num->impl.natural_num = array;
    num->str = NULL;
    num->type = TXC_NATURAL_NUM;
    num->singleton = false;
    return num;
}

/* NUM */

static void txc_num_assert_valid(const txc_num *const num)
{
    assert(num != NULL);
    if (num->singleton)
    {
        assert(num->str != NULL);
    }
    switch (num->type)
    {
    case TXC_NAN: /* FALLTHROUGH */
    case TXC_NIL:
        break;
    case TXC_ZERO:
        assert(num == &TXC_ZERO_ZERO);
        break;
    case TXC_NATURAL_NUM:
        txc_num_array_assert_valid(num->impl.natural_num);
        break;
    default:
        assert(false);
        break;
    }
}

const char *txc_num_to_str(txc_num *const num)
{
    txc_num_assert_valid(num);
    if (num->str != NULL)
        return num->str;
    switch (num->type)
    {
    case TXC_NAN:
        num->str = TXC_NAN_UNSPECIFIED.str;
        return TXC_NAN_UNSPECIFIED.str;
    case TXC_NIL:
        num->str = TXC_NIL_UNSPECIFIED.str;
        return TXC_NIL_UNSPECIFIED.str;
    case TXC_ZERO:
        num->str = TXC_ZERO_ZERO.str;
        return TXC_ZERO_ZERO.str;
    case TXC_NATURAL_NUM:
    {
        struct txc_num_array array = *(num->impl.natural_num);
        const size_t max_len = TXC_NUM_ARRAY_TYPE_WIDTH / CHAR_BIT * 3 * array.used + 1;
        char *str = malloc(max_len);
        if (str == NULL)
        {
            fprintf(stderr, TXC_ERROR_ALLOC, max_len, "natural number decimal string", __FILE__, __LINE__);
            return TXC_NAN_ERROR_ALLOC.str;
        }
        size_t len = 0;
        for (size_t array_i = array.used; array_i > 0; array_i--)
        {
            for (size_t bit_i = TXC_NUM_ARRAY_TYPE_WIDTH; bit_i > 0; bit_i--)
            {
                uint_fast8_t carry = (array.data[array_i - 1] >> (bit_i - 1)) & 1;
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
        num->str = str;
        return str;
    }
    default:
        fprintf(stderr, TXC_ERROR_UNKNOWN_NUM_TYPE, num->type, __FILE__, __LINE__);
        return TXC_PRINT_ERROR_UNKNOWN_NUM_TYPE;
    }
}
