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

#include <stdio.h>
#include <string.h>

#include "integer.h"
#include "node.h"

#define TEST(test)                        \
    else if (strcmp(argv[1], #test) == 0) \
        ec = test();

/* TEST */

static int test_start(void)
{
    printf("Running tests for TeXalC %u.%u.%u\n", TXC_VERSION_MAJOR, TXC_VERSION_MINOR, TXC_VERSION_PATCH);
    return -3;
}

static int test_test(void)
{
    return 0;
}

static int test_finish(void)
{
    printf("Finished Tests\n");
    return -3;
}

/* INTEGER */

static int integer_is(const txc_int *const integer, uint_fast8_t flags)
{
    if (!txc_int_test_valid(integer))
        return 1;
    if (((flags & 1) != 0) != txc_int_is_pos_one(integer))
        return 2;
    if (((flags & 2) != 0) != txc_int_is_zero(integer))
        return 3;
    if (((flags & 4) != 0) != txc_int_is_neg_one(integer))
        return 4;
    if (((flags & 8) != 0) != txc_int_is_neg(integer))
        return 5;
    return 0;
}

static int integer_one(void)
{
    txc_int *one = txc_int_create_one();
    uint_fast8_t ec = integer_is(one, 1);
    if (ec != 0)
        return ec;
    one = txc_int_neg(one);
    ec = integer_is(one, 12);
    if (ec != 0)
        return ec;
    txc_int_free(one);
    return 0;
}

static int integer_zero(void)
{
    txc_int *zero = txc_int_create_zero();
    uint_fast8_t ec = integer_is(zero, 2);
    if (ec != 0)
        return ec;
    ec = integer_is(zero, 2);
    if (ec != 0)
        return ec;
    txc_int_free(zero);
    return 0;
}

static int integer_create(const txc_node *a, const txc_node *b)
{
    if (!txc_node_test_valid(a, true))
        return 1;
    if (!txc_node_test_valid(b, true))
        return 2;
    txc_int *ai = txc_int_copy(txc_node_to_int(a));
    txc_int *bi = txc_int_copy(txc_node_to_int(b));
    if (!txc_int_test_valid(ai))
        return 3;
    if (!txc_int_test_valid(bi))
        return 4;
    if (txc_int_cmp(ai, txc_node_to_int(a)) != 0)
        return 5;
    if (txc_int_cmp_abs(txc_node_to_int(b), bi) != 0)
        return 6;
    txc_node_free(a);
    txc_node_free(b);
    if (txc_int_cmp(ai, bi) <= 0)
        return 7;
    if (txc_int_cmp_abs(bi, ai) >= 0)
        return 8;
    ai = txc_int_neg(ai);
    bi = txc_int_neg(txc_int_neg(bi));
    if (!txc_int_test_valid(ai))
        return 9;
    if (!txc_int_test_valid(bi))
        return 10;
    if (txc_int_cmp(bi, ai) >= 0)
        return 11;
    if (txc_int_cmp_abs(ai, bi) <= 0)
        return 12;
    txc_int_free(ai);
    txc_int_free(bi);
    return 0;
}

static int integer_create_bin(void)
{
    return integer_create(
        txc_int_create_int_node("000000000000000000000000000000000011111111111111111111111111111111111111111100010011001110011111111000110100110001100100111101110010010001000101011111111110101100010111001010110101110001000001110111101010011110010101110011010", 225, 2),
        txc_int_create_int_node("000000000000000000000000000000000011111111111111111111111111111111111111111000110110111000001101111011110011001100111011001001101100010100010101010000101000000110101100111100011110111110011110100101110011101100100111100001100", 225, 2));
}

static int integer_create_dec(void)
{
    return integer_create(
        txc_int_create_int_node("00000000000000012345678999999999999999999999999999957828869979142051649751720016828691859419069997121852344624842547461911270140479352056870471", 143, 10),
        txc_int_create_int_node("00000000000000012345678999999999999999999999999999931741352887315442455039904720810482826387864888317910827616021764335832460327395743586003187", 143, 10));
}

static int integer_create_hex(void)
{
    return integer_create(
        txc_int_create_int_node("000000000000000000000111111112222222233333333444444445555555566666666777777778888888899999999AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDEEEEEEEEFFFFFFFF10ECFA2A5E20228613CA0B138EA6A3F089EDB0F27640E544BCD1CC58DDE0D985EBC928D1B7744", 218, 16),
        txc_int_create_int_node("000000000000000000000111111112222222233333333444444445555555566666666777777778888888899999999aaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff0ea7d14f8aeca054baa8591d88eb0590fa85f0c2ccbfe77d106cd07faec9f2995dd0b22f3eeb0", 218, 16));
}

/* MAIN */

int main(int argc, char **argv)
{
    int ec;
    if (argc < 2) {
        fprintf(stderr, "No test name specified.\n");
        return -1;
    }
    TEST(test_start)
    TEST(test_test)
    TEST(integer_one)
    TEST(integer_zero)
    TEST(integer_create_bin)
    TEST(integer_create_dec)
    TEST(integer_create_hex)
    TEST(test_finish)
    else
    {
        fprintf(stderr, "No test with name %s exists.\n", argv[1]);
        return -2;
    }
    switch (ec) {
    case -3:
        ec = 0;
        break;
    case 0:
        printf("PASS: %s\n", argv[1]);
        break;
    default:
        printf("FAIL: %s with exit code %d\n", argv[1], ec);
        break;
    }
    return ec;
}
