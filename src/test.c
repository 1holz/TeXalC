/*
 *     Copyright (C) 2024 - 2025  Einholz
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
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "integer.h"
#include "node.h"
#include "parser.h"
#include "test.h"
#include "util.h"

#define TEST(test)                        \
    else if (strcmp(argv[1], #test) == 0) \
        ec = fork_test(&test);

#ifndef TXC_BUF_SIZE
#define TXC_BUF_SIZE 1024
#endif /* TXC_BUF_SIZE */

#define ZERO "0"
#define TWO "2"
#define BIN_1 "000000000000000000000000000000000011111111111111111111111111111111111111111100010011001110011111111000110100110001100100111101110010010001000101011111111110101100010111001010110101110001000001110111101010011110010101110011010"
#define BIN_2 "000000000000000000000000000000000011111111111111111111111111111111111111111000110110111000001101111011110011001100111011001001101100010100010101010000101000000110101100111100011110111110011110100101110011101100100111100001100"
#define DEC_1 "00000000000000012345678999999999999999999999999999957828869979142051649751720016828691859419069997121852344624842547461911270140479352056870471"
#define DEC_2 "00000000000000012345678999999999999999999999999999931741352887315442455039904720810482826387864888317910827616021764335832460327395743586003187"
#define HEX_1 "000000000000000000000111111112222222233333333444444445555555566666666777777778888888899999999AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDDEEEEEEEEFFFFFFFF10ECFA2A5E20228613CA0B138EA6A3F089EDB0F27640E544BCD1CC58DDE0D985EBC928D1B7744"
#define HEX_2 "000000000000000000000111111112222222233333333444444445555555566666666777777778888888899999999aaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff0ea7d14f8aeca054baa8591d88eb0590fa85f0c2ccbfe77d106cd07faec9f2995dd0b22f3eeb0"
#define LONG_1 "00000000000000000000011111111222222223333333344444444555555556666666677777777888888889999999922814505899001880041123880539161321315269426251213954488098655535479423803963573489947024645657402074154360929407451092662273279888758893524036770493141619090"
#define LONG_2 "00000000000000000000011111111222222223333333344444444555555556666666677777777888888889999999949379366806399785369702180938850690083950083692313492029373987486480856937804678660135369309547184217291540356840132191226398398000148320073212339532302790867"
#define LONG_3 "00000000000000000000011111111222222223333333344444444555555556666666677777777888888889999999935875564840971245001735018636875701845398144277921533847990349893812898901745048341405675013642066881517753219316780321569245150928062216639597697391788989218"
#define LONG_4 "00000000000000000000011111111222222223333333344444444555555556666666677777777888888889999999946491720489394233566925730448224076264819411866441133015109101298731497051922205944184631691956431918443405161201462693148860904831515718324001845989813702186"
#define LONG_5 "00000000000000000000011111111222222223333333344444444555555556666666677777777888888889999999933075244988108187282275739527416611987733859948573460000358340975004725238950900773643174529467627350172631392980271317548260752490925868333811864959678661031"
#define UNSIGNED_ADD_SOL "55555556111111116666666722222222777777783333333388888889444444449999999687636403023875331261762550090528401497170926036463573380930435189509401934386407209315875190270712441579691059746097616155038486139411016894660518366725762392"
#define SIGNED_ADD_SOL "11111111222222223333333344444444555555556666666677777777888888889999999895894228432287293388506727316378868799631934918954323291964257619084693954932638000675873187263480170109800023662907847404519880476082940100232147322492776286"
#define MUL_SOL "169350886551847025351832376500990702638926992872021541990550229385764480666443954620727122692539576421015350243734673536666653866729396898201004310585209594435286810069303568709210557326982963130851676763413581558443578795873115592910919779691911148276175667700944954397952321203102794641797091078746751182609160188237766491908753577450918900539229077851525582987958024530931747900325877049613035287995732265235660054809402532628592977406438503340547869801425036332338671990845012323610950305038127153995675360977548949584684778565074291984307606257594500433108529378185427348210348209590545241596666550329727911125652169401277956953386881018821711265407436265166815012041919282718363116717271448004304277408276100601462379515523612833354978056982494621003257578343632488899292157630632065255366196500859562680381003219723172796429522542662261391408826326384256351949283574595399553665260238357077597088374941062575920265619201357183467154234763614992374494943445825977845487436176364888647334729213371656976047160709297372472531171578789649078543127709608878474182093837204987832901201341261802484118524255098304545934285828335006845456312037640"
#define NEG_MUL_SOL "(-169350886551847025351832376500990702638926992872021541990550229385764480666443954620727122692539576421015350243734673536666653866729396898201004310585209594435286810069303568709210557326982963130851676763413581558443578795873115592910919779691911148276175667700944954397952321203102794641797091078746751182609160188237766491908753577450918900539229077851525582987958024530931747900325877049613035287995732265235660054809402532628592977406438503340547869801425036332338671990845012323610950305038127153995675360977548949584684778565074291984307606257594500433108529378185427348210348209590545241596666550329727911125652169401277956953386881018821711265407436265166815012041919282718363116717271448004304277408276100601462379515523612833354978056982494621003257578343632488899292157630632065255366196500859562680381003219723172796429522542662261391408826326384256351949283574595399553665260238357077597088374941062575920265619201357183467154234763614992374494943445825977845487436176364888647334729213371656976047160709297372472531171578789649078543127709608878474182093837204987832901201341261802484118524255098304545934285828335006845456312037640)"
#define DIVISOR "2688811131697455799312686635590409314476653708480527751110523555767208122010442771791153338445280213913034380290994023971"
#define DIV_SOL "4591501"
#define REASON_DUMMY "dummy reason"
#define NAN_UNSPECIFIED "\\text{NAN(unspecified)}"
#define NAN_DUMMY "\\text{NAN(dummy reason)}"

/* DEFINITIONS */

enum op {
    UNSIGNED_ADD,
    SIGNED_ADD,
    UNSIGNED_MUL,
    SIGNED_MUL
};

/* SETUP */

// Assumes being called in its own process
static int parser(const char *const input, const char *const expected)
{
    const size_t close_fds_size = 6;
    int close_fds[close_fds_size];
    memset(close_fds, -1, sizeof close_fds);
    const size_t undup_fds_size = 2;
    int undup_fds[2 * undup_fds_size];
    memset(undup_fds, -1, sizeof undup_fds);
    size_t to_read = txc_max(TXC_BUF_SIZE, strlen(expected));
    char buf[2 * to_read];
    memset(buf, 0, 2 * to_read);
    const int old_stdin = dup(STDIN_FILENO);
    if (old_stdin < 0) {
        TXC_TEST_ERROR_DUP(STDIN_FILENO);
        goto error_cleanup;
    }
    close_fds[0] = old_stdin;
    const int old_stdout = dup(STDOUT_FILENO);
    if (old_stdin < 0) {
        TXC_TEST_ERROR_DUP(STDOUT_FILENO);
        goto error_cleanup;
    }
    close_fds[1] = old_stdout;
    int i_fd[2];
    int o_fd[2];
    const int i_pipe_res = pipe(i_fd);
    if (i_pipe_res != 0) {
        TXC_TEST_ERROR_PIPE();
        goto error_cleanup;
    }
    close_fds[2] = i_fd[0];
    close_fds[3] = i_fd[1];
    const int o_pipe_res = pipe(o_fd);
    if (o_pipe_res != 0) {
        TXC_TEST_ERROR_PIPE();
        goto error_cleanup;
    }
    close_fds[4] = o_fd[0];
    close_fds[5] = o_fd[1];
    const int i_dup_res = dup2(i_fd[0], STDIN_FILENO);
    if (i_dup_res < 0) {
        TXC_TEST_ERROR_DUP2(i_fd[0], STDIN_FILENO);
        goto error_cleanup;
    }
    undup_fds[0] = old_stdin;
    undup_fds[1] = STDIN_FILENO;
    const int i0_close_res = close(i_fd[0]);
    if (i0_close_res != 0) {
        TXC_TEST_ERROR_CLOSE(i_fd[0]);
        goto error_cleanup;
    }
    close_fds[2] = -1;
    const int o_dup_res = dup2(o_fd[1], STDOUT_FILENO);
    if (o_dup_res < 0) {
        TXC_TEST_ERROR_DUP2(o_fd[1], STDOUT_FILENO);
        goto error_cleanup;
    }
    undup_fds[2] = old_stdout;
    undup_fds[3] = STDOUT_FILENO;
    const int o1_close_res = close(o_fd[1]);
    if (o1_close_res != 0) {
        TXC_TEST_ERROR_CLOSE(o_fd[1]);
        goto error_cleanup;
    }
    close_fds[5] = -1;
    // TODO include zero byte?
    size_t remaining_write = strlen(input);
    size_t written = 0;
    while (written < remaining_write) {
        ssize_t write_successful = write(i_fd[1], input + written, remaining_write);
        if (write_successful < 0) {
            TXC_TEST_ERROR_WRITE(remaining_write, i_fd[1]);
            goto error_cleanup;
        }
        written += write_successful;
        remaining_write -= write_successful;
    }
    const int i1_close_res = close(i_fd[1]);
    if (i1_close_res != 0) {
        TXC_TEST_ERROR_CLOSE(i_fd[1]);
        goto error_cleanup;
    }
    close_fds[3] = -1;
    yyparse();
    const int i_flush_res = fflush(stdin);
    if (i_flush_res != 0)
        TXC_TEST_WARN_FFLUSH();
    const int o_flush_res = fflush(stdout);
    if (o_flush_res != 0)
        TXC_TEST_WARN_FFLUSH();
    const int i_close_res = close(STDIN_FILENO);
    if (i_close_res != 0) {
        TXC_TEST_ERROR_CLOSE(STDIN_FILENO);
        goto error_cleanup;
    }
    const int o_close_res = close(STDOUT_FILENO);
    if (o_close_res != 0) {
        TXC_TEST_ERROR_CLOSE(STDOUT_FILENO);
        goto error_cleanup;
    }
    size_t total_read = 0;
    // to_read and buf declaration could be here but would bypass goto
    ssize_t read_successful = read(o_fd[0], &buf[to_read], to_read);
    while (read_successful > 0) {
        memmove(&buf[0], &buf[read_successful], to_read);
        total_read += read_successful;
        read_successful = read(o_fd[0], &buf[to_read], to_read);
    }
    if (read_successful < 0) {
        TXC_TEST_ERROR_READ(to_read, o_fd[0]);
        goto error_cleanup;
    }
    buf[to_read] = 0;
    const int o0_close_res = close(o_fd[0]);
    if (o0_close_res != 0) {
        TXC_TEST_ERROR_CLOSE(o_fd[0]);
        goto error_cleanup;
    }
    close_fds[4] = -1;
    const int i_reset_res = dup2(old_stdin, STDIN_FILENO);
    if (i_reset_res < 0) {
        TXC_TEST_ERROR_DUP2(old_stdin, STDIN_FILENO);
        goto error_cleanup;
    }
    undup_fds[0] = -1;
    undup_fds[1] = -1;
    const int o_reset_res = dup2(old_stdout, STDOUT_FILENO);
    if (o_reset_res < 0) {
        TXC_TEST_ERROR_DUP2(old_stdout, STDOUT_FILENO);
        goto error_cleanup;
    }
    undup_fds[2] = -1;
    undup_fds[3] = -1;
    if (strncmp(&buf[to_read - strlen(expected)], expected, strlen(expected)) != 0) {
        size_t cmp_start = to_read < total_read ? 0 : to_read - total_read;
        fprintf(stderr, "Got:\n%s\nExpected:\n%s\n", &buf[cmp_start], expected);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
error_cleanup:
    for (size_t i = 0; i < close_fds_size; i++) {
        if (close_fds[i] < 0)
            continue;
        const int close_res = close(close_fds[i]);
        if (close_res != 0)
            TXC_TEST_WARN_CLOSE(close_fds[i]);
    }
    for (size_t i = 0; i + 1 < undup_fds_size; i += 2) {
        if (undup_fds[i] < 0 || undup_fds[i + 1] < 0)
            continue;
        const int dup_res = dup2(undup_fds[i], undup_fds[i + 1]);
        if (dup_res < 0)
            TXC_TEST_WARN_DUP2(undup_fds[i], undup_fds[i + 1]);
        const int close_res = close(undup_fds[i]);
        if (close_res != 0)
            TXC_TEST_WARN_CLOSE(undup_fds[i]);
    }
    exit(EXIT_FAILURE);
}

static int fork_test(void(test)(void))
{
    const int cpid = fork();
    if (cpid < 0) {
        // error
        TXC_TEST_ERROR_FORK();
        return -1;
    } else if (cpid == 0) {
        test();
        exit(EXIT_SUCCESS);
    }
    // parent
    // TODO add timeout
    int status;
    waitpid(cpid, &status, 0);
    if (!WIFEXITED(status)) {
        TXC_TEST_ERROR_ABNORMAL();
        return -2;
    }
    if (WEXITSTATUS(status) != 0)
        return WEXITSTATUS(status);
    return EXIT_SUCCESS;
}

/* TEST */

static void test_test(void)
{
    exit(EXIT_SUCCESS);
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

static void integer_one(void)
{
    txc_int *one = txc_int_create_one();
    uint_fast8_t ec = integer_is(one, 1); // 0b0001
    if (ec != 0)
        exit(ec);
    one = txc_int_neg(one);
    ec = integer_is(one, 12); // 0b1100
    if (ec != 0)
        exit(ec);
    txc_int_free(one);
    exit(EXIT_SUCCESS);
}

static void integer_zero(void)
{
    txc_int *zero = txc_int_create_zero();
    uint_fast8_t ec = integer_is(zero, 2); // 0b0010
    if (ec != 0)
        exit(ec);
    ec = integer_is(zero, 2); // 0b0010
    if (ec != 0)
        exit(ec);
    txc_int_free(zero);
    exit(EXIT_SUCCESS);
}

static void integer_copy(void)
{
    if (txc_int_copy(NULL) != NULL)
        exit(1);
    int ec = 0;
    txc_node *const node = txc_int_create_int_node(MUL_SOL, strlen(MUL_SOL), 10);
    if (!txc_node_test_valid(node, true)) {
        ec = 2;
        goto clean_node_only;
    }
    const txc_int *const int_1 = txc_int_copy(txc_node_to_int(node));
    const txc_int *const int_2 = txc_int_copy(int_1);
    if (!txc_int_test_valid(int_1) || !txc_int_test_valid(int_2)) {
        ec = 3;
        goto clean_node;
    }
    txc_node_free(node);
    if (txc_int_cmp(int_1, int_2) != 0) {
        ec = 4;
        goto clean_1;
    }
    txc_int_free(int_1);
    if (!txc_int_test_valid(int_2)) {
        ec = 5;
        goto clean_2;
    }
    goto clean_2;
clean_node:
    txc_node_free(node);
clean_1:
    txc_int_free(int_1);
clean_2:
    txc_int_free(int_2);
    exit(ec);
clean_node_only:
    txc_node_free(node);
    exit(ec);
}

static int integer_create(txc_node *a, txc_node *b)
{
    if (txc_int_neg(NULL) != NULL)
        return 1;
    int ec = 0;
    if (!txc_node_test_valid(a, true) || !txc_node_test_valid(b, true)) {
        ec = 2;
        goto clean_nodes;
    }
    txc_int *ai = txc_int_copy(txc_node_to_int(a));
    txc_int *bi = txc_int_copy(txc_node_to_int(b));
    if (!txc_int_test_valid(ai) || !txc_int_test_valid(bi)) {
        ec = 3;
        goto clean_both;
    }
    if (txc_int_cmp(ai, txc_node_to_int(a)) != 0) {
        ec = 4;
        goto clean_both;
    }
    if (txc_int_cmp_abs(txc_node_to_int(b), bi) != 0) {
        ec = 5;
        goto clean_both;
    }
    txc_node_free(a);
    txc_node_free(b);
    if (txc_int_cmp(ai, bi) <= 0) {
        ec = 6;
        goto clean_ints;
    }
    if (txc_int_cmp_abs(bi, ai) >= 0) {
        ec = 7;
        goto clean_ints;
    }
    ai = txc_int_neg(ai);
    bi = txc_int_neg(txc_int_neg(bi));
    if (!txc_int_test_valid(ai) || !txc_int_test_valid(bi)) {
        ec = 8;
        goto clean_ints;
    }
    if (txc_int_cmp(bi, ai) >= 0) {
        ec = 9;
        goto clean_ints;
    }
    if (txc_int_cmp_abs(ai, bi) <= 0) {
        ec = 10;
        goto clean_ints;
    }
    txc_int_free(ai);
    txc_int_free(bi);
    return ec;
clean_nodes:
    txc_node_free(a);
    txc_node_free(b);
    return ec;
clean_both:
    txc_node_free(a);
    txc_node_free(b);
clean_ints:
    txc_int_free(ai);
    txc_int_free(bi);
    return ec;
}

static void integer_create_bin(void)
{
    exit(integer_create(
        txc_int_create_int_node(BIN_1, 225, 2),
        txc_int_create_int_node(BIN_2, 225, 2)));
}

static void integer_create_dec(void)
{
    exit(integer_create(
        txc_int_create_int_node(DEC_1, 143, 10),
        txc_int_create_int_node(DEC_2, 143, 10)));
}

static void integer_create_hex(void)
{
    exit(integer_create(
        txc_int_create_int_node(HEX_1, 218, 16),
        txc_int_create_int_node(HEX_2, 218, 16)));
}

static int integer_add_mul(const enum op op, const char *const operand_strs[5], const char *const sol_str)
{
    if (txc_int_neg(NULL) != NULL)
        return 1;
    const bool neg = op == SIGNED_ADD || op == SIGNED_MUL;
    const bool mul = op == UNSIGNED_MUL || op == SIGNED_MUL;
    int ec = 0;
    txc_node *const operand_nodes[5] = {
        txc_int_create_int_node(operand_strs[0], strlen(operand_strs[0]), 10),
        txc_int_create_int_node(operand_strs[1], strlen(operand_strs[1]), 10),
        txc_int_create_int_node(operand_strs[2], strlen(operand_strs[2]), 10),
        txc_int_create_int_node(operand_strs[3], strlen(operand_strs[3]), 10),
        txc_int_create_int_node(operand_strs[4], strlen(operand_strs[4]), 10)
    };
    for (size_t i = 0; i < 5; i++) {
        if (txc_node_test_valid(operand_nodes[i], true))
            continue;
        ec = 2;
        goto clean;
    }
    const txc_int *const operands[5] = {
        neg ? txc_int_neg((txc_int *)txc_node_to_int(operand_nodes[0])) : txc_node_to_int(operand_nodes[0]),
        txc_node_to_int(operand_nodes[1]),
        neg ? txc_int_neg((txc_int *)txc_node_to_int(operand_nodes[2])) : txc_node_to_int(operand_nodes[2]),
        txc_node_to_int(operand_nodes[3]),
        neg ? txc_int_neg((txc_int *)txc_node_to_int(operand_nodes[4])) : txc_node_to_int(operand_nodes[4])
    };
    for (size_t i = 0; i < 5; i++) {
        if (txc_int_test_valid(operands[i]))
            continue;
        ec = 3;
        goto clean;
    }
    txc_node *const sol_node = txc_int_create_int_node(sol_str, strlen(sol_str), 10);
    if (!txc_node_test_valid(sol_node, true)) {
        ec = 4;
        goto clean;
    }
    const txc_int *const solution = neg ? txc_int_neg((txc_int *)txc_node_to_int(sol_node)) : txc_node_to_int(sol_node);
    if (!txc_int_test_valid(solution)) {
        ec = 5;
        goto clean;
    }
    const txc_int *result;
    if (mul) {
        if (!txc_int_is_pos_one(txc_int_mul(NULL, 0))) {
            ec = 6;
            goto clean;
        }
        if (txc_int_mul(NULL, 1) != NULL) {
            ec = 7;
            goto clean;
        }
        result = txc_int_mul(operands, 5);
    } else {
        if (!txc_int_is_zero(txc_int_add(NULL, 0))) {
            ec = 6;
            goto clean;
        }
        if (txc_int_add(NULL, 1) != NULL) {
            ec = 7;
            goto clean;
        }
        result = txc_int_add(operands, 5);
    }
    if (!txc_int_test_valid(result)) {
        ec = 8;
        goto clean_result;
    }
    if (txc_int_cmp(solution, result) != 0) {
        ec = 9;
        goto clean_result;
    }
clean_result:
    txc_int_free(result);
clean:
    txc_node_free(sol_node);
    for (size_t i = 0; i < 5; i++)
        txc_node_free(operand_nodes[i]);
    return ec;
}

static void integer_unsigned_add(void)
{
    const char *const operands_node[5] = { LONG_1, LONG_2, LONG_3, LONG_4, LONG_5 };
    exit(integer_add_mul(UNSIGNED_ADD, operands_node, UNSIGNED_ADD_SOL));
}

static void integer_signed_add(void)
{
    const char *const operands_node[5] = { LONG_1, LONG_2, LONG_3, LONG_4, LONG_5 };
    exit(integer_add_mul(SIGNED_ADD, operands_node, SIGNED_ADD_SOL));
}

static void integer_unsigned_mul(void)
{
    const char *const operands_node_1[5] = { LONG_1, LONG_2, LONG_3, LONG_4, LONG_5 };
    const int ec = integer_add_mul(UNSIGNED_MUL, operands_node_1, MUL_SOL);
    if (ec != 0)
        exit(ec);
    const char *const operands_node_2[5] = { MUL_SOL, LONG_2, LONG_3, ZERO, LONG_5 };
    exit(integer_add_mul(UNSIGNED_MUL, operands_node_2, ZERO));
}

static void integer_signed_mul(void)
{
    const char *const operands_node_1[5] = { LONG_1, LONG_2, LONG_3, LONG_4, LONG_5 };
    const int ec = integer_add_mul(SIGNED_MUL, operands_node_1, MUL_SOL);
    if (ec != 0)
        exit(ec);
    const char *const operands_node_2[5] = { MUL_SOL, LONG_2, LONG_3, ZERO, LONG_5 };
    exit(integer_add_mul(SIGNED_MUL, operands_node_2, ZERO));
}

static void integer_gcd(void)
{
    if (!txc_int_is_pos_one(txc_int_mul(NULL, 0)))
        exit(1);
    if (txc_int_mul(NULL, 1) != NULL)
        exit(2);
    int ec = 0;
    txc_node *const gcd_node = txc_int_create_int_node(MUL_SOL, strlen(MUL_SOL), 10);
    txc_node *const two_node = txc_int_create_int_node(TWO, strlen(TWO), 10);
    txc_node *const a_node = txc_int_create_int_node(UNSIGNED_ADD_SOL, strlen(UNSIGNED_ADD_SOL), 10);
    txc_node *const b_node = txc_int_create_int_node(SIGNED_ADD_SOL, strlen(SIGNED_ADD_SOL), 10);
    if (!txc_node_test_valid(gcd_node, true) || !txc_node_test_valid(a_node, true) || !txc_node_test_valid(b_node, true)) {
        ec = 3;
        goto clean_nodes;
    }
    const txc_int *const proto_gcd[2] = { txc_node_to_int(gcd_node), txc_node_to_int(two_node) };
    const txc_int *const a[2] = { txc_node_to_int(a_node), proto_gcd[0] };
    const txc_int *const b[2] = { txc_node_to_int(b_node), proto_gcd[0] };
    if (!txc_int_test_valid(proto_gcd[0]) || !txc_int_test_valid(proto_gcd[1]) || !txc_int_test_valid(a[0]) || !txc_int_test_valid(b[0])) {
        ec = 4;
        goto clean_nodes;
    }
    const txc_int *const gcd = txc_int_mul(proto_gcd, 2);
    const txc_int *const zero = txc_int_create_zero();
    const txc_int *const a_mul = txc_int_mul(a, 2);
    const txc_int *const b_mul = txc_int_mul(b, 2);
    if (!txc_int_test_valid(gcd) || !txc_int_test_valid(zero) || !txc_int_test_valid(a_mul) || !txc_int_test_valid(b_mul)) {
        ec = 5;
        goto clean_muls;
    }
    if (txc_int_gcd(NULL, zero) != NULL || txc_int_gcd(zero, NULL) != NULL) {
        ec = 6;
        goto clean_muls;
    }
    const txc_int *const result_big = txc_int_gcd(a_mul, b_mul);
    const txc_int *const result_zero_1 = txc_int_gcd(zero, gcd);
    const txc_int *const result_zero_2 = txc_int_gcd(gcd, zero);
    if (!txc_int_test_valid(result_big) || !txc_int_test_valid(result_zero_1) || !txc_int_test_valid(result_zero_2)) {
        ec = 7;
        goto clean_result;
    }
    if (txc_int_cmp(result_big, gcd) != 0) {
        ec = 8;
        goto clean_result;
    }
    if (txc_int_cmp(result_zero_1, gcd) != 0) {
        ec = 9;
        goto clean_result;
    }
    if (txc_int_cmp(result_zero_2, gcd) != 0) {
        ec = 10;
        goto clean_result;
    }
clean_result:
    txc_int_free(result_big);
    txc_int_free(result_zero_1);
    txc_int_free(result_zero_2);
clean_muls:
    txc_int_free(gcd);
    txc_int_free(zero);
    txc_int_free(a_mul);
    txc_int_free(b_mul);
clean_nodes:
    txc_node_free(gcd_node);
    txc_node_free(two_node);
    txc_node_free(a_node);
    txc_node_free(b_node);
    exit(ec);
}

static void integer_div_invalid(void)
{
    int ec = 0;
    const txc_int *zero = txc_int_create_zero();
    if (!txc_int_test_valid(zero)) {
        ec = 1;
        goto clean;
    }
    if (txc_int_div(NULL, zero) != NULL) {
        ec = 2;
        goto clean;
    }
    if (txc_int_div(zero, NULL) != NULL) {
        ec = 3;
        goto clean;
    }
clean:
    txc_int_free(zero);
    exit(ec);
}

static void integer_div(void)
{
    int ec = 0;
    txc_node *const prod_node = txc_int_create_int_node(DEC_1, strlen(DEC_1), 10);
    txc_node *const div_node = txc_int_create_int_node(DIVISOR, strlen(DIVISOR), 10);
    txc_node *const sol_node = txc_int_create_int_node(DIV_SOL, strlen(DIV_SOL), 10);
    if (!txc_node_test_valid(prod_node, true) || !txc_node_test_valid(div_node, true) || !txc_node_test_valid(sol_node, true)) {
        ec = 1;
        goto clean_input;
    }
    const txc_int *prod = txc_node_to_int(prod_node);
    const txc_int *const div = txc_node_to_int(div_node);
    const txc_int *const sol = txc_node_to_int(sol_node);
    if (!txc_int_test_valid(prod) || !txc_int_test_valid(div) || !txc_int_test_valid(sol)) {
        ec = 2;
        goto clean;
    }
    const txc_int *tmp = txc_int_div(prod, div);
    txc_node_free(prod_node);
    prod = tmp;
    if (!txc_int_test_valid(tmp)) {
        ec = 3;
        goto clean;
    }
    tmp = txc_int_div(prod, sol);
    txc_int_free(prod);
    prod = tmp;
    if (!txc_int_test_valid(tmp)) {
        ec = 4;
        goto clean;
    }
    if (!txc_int_is_pos_one(prod)) {
        ec = 5;
        goto clean;
    }
clean:
    txc_int_free(prod);
clean_input:
    txc_node_free(div_node);
    txc_node_free(sol_node);
    exit(ec);
}

static void integer_to_str(void)
{
    if (txc_int_to_str(NULL) != NULL)
        exit(1);
    int ec = 0;
    txc_node *const large_node = txc_int_create_int_node(MUL_SOL, strlen(MUL_SOL), 10);
    if (!txc_node_test_valid(large_node, true)) {
        ec = 2;
        goto clean_node;
    }
    const txc_int *const large = txc_node_to_int(large_node);
    const txc_int *const zero = txc_int_create_zero();
    if (!txc_int_test_valid(large) || !txc_int_test_valid(zero)) {
        ec = 3;
        goto clean_zero;
    }
    char *const large_str = txc_int_to_str(large);
    if (large_str == NULL) {
        ec = 4;
        goto clean_zero;
    }
    char *const zero_str = txc_int_to_str(zero);
    if (zero_str == NULL) {
        ec = 5;
        goto clean_large_str;
    }
    if (strcmp(large_str, MUL_SOL) != 0) {
        ec = 6;
        goto clean_zero_str;
    }
    if (strcmp(zero_str, ZERO) != 0) {
        ec = 7;
        goto clean_zero_str;
    }
clean_zero_str:
    free(zero_str);
clean_large_str:
    free(large_str);
clean_zero:
    txc_int_free(zero);
clean_node:
    txc_node_free(large_node);
    exit(ec);
}

/* NODE */

static void node_constants(void)
{
    txc_node_free(&TXC_NAN_ERROR_ALLOC);
    txc_node_free(&TXC_NAN_ERROR_INVALID_NODE_TYPE);
    txc_node_free(&TXC_NAN_ERROR_OVERFLOW);
    txc_node_free(&TXC_NAN_ERROR_NYI);
    txc_node_free(&TXC_NAN_ERROR_ZERO_DIVISION);
    txc_node_free(&TXC_NAN_UNSPECIFIED);
    if (!txc_node_test_valid(&TXC_NAN_ERROR_ALLOC, true))
        exit(1);
    if (!txc_node_test_valid(&TXC_NAN_ERROR_INVALID_NODE_TYPE, true))
        exit(2);
    if (!txc_node_test_valid(&TXC_NAN_ERROR_OVERFLOW, true))
        exit(3);
    if (!txc_node_test_valid(&TXC_NAN_ERROR_NYI, true))
        exit(4);
    if (!txc_node_test_valid(&TXC_NAN_ERROR_ZERO_DIVISION, true))
        exit(5);
    if (!txc_node_test_valid(&TXC_NAN_UNSPECIFIED, true))
        exit(6);
    exit(EXIT_SUCCESS);
}

static void node_create_nan(void)
{
    const txc_node *const unspecified = txc_node_create_nan(NULL);
    const txc_node *const dummy = txc_node_create_nan(REASON_DUMMY);
    if (!txc_node_test_valid(unspecified, true) || !txc_node_test_valid(dummy, true))
        exit(1);
    char *const unspecified_str = txc_node_to_str(unspecified);
    char *const dummy_str = txc_node_to_str(dummy);
    if (strcmp(unspecified_str, NAN_UNSPECIFIED) != 0)
        exit(2);
    if (strcmp(dummy_str, NAN_DUMMY) != 0)
        exit(3);
    txc_node_free(unspecified);
    txc_node_free(dummy);
    free(unspecified_str);
    free(dummy_str);
    exit(EXIT_SUCCESS);
}

static void node_neg(void)
{
    txc_node *const node_int = txc_int_create_int_node(MUL_SOL, strlen(MUL_SOL), 10);
    txc_node *const node_neg = txc_node_create_un_op(TXC_NEG, node_int);
    if (!txc_node_test_valid(node_int, true) || !txc_node_test_valid(node_neg, true))
        exit(1);
    txc_node *const node_res = txc_node_simplify(node_neg);
    txc_node_free(node_neg);
    if (!txc_node_test_valid(node_res, true))
        exit(2);
    char *const str = txc_node_to_str(node_res);
    if (strcmp(str, NEG_MUL_SOL) != 0)
        exit(3);
    free(str);
    if (!txc_int_is_neg(txc_node_to_int(node_res)))
        exit(4);
    txc_node_free(node_res);
    exit(EXIT_SUCCESS);
}

static int node_add_mul(const enum op op, const char *const operand_strs[5], const char *const sol_str)
{
    if (txc_int_neg(NULL) != NULL)
        return 1;
    const bool neg = op == SIGNED_ADD || op == SIGNED_MUL;
    const enum txc_node_type type = op == UNSIGNED_MUL || op == SIGNED_MUL ? TXC_MUL : TXC_ADD;
    txc_node *op_nodes[5] = {
        txc_int_create_int_node(operand_strs[0], strlen(operand_strs[0]), 10),
        txc_int_create_int_node(operand_strs[1], strlen(operand_strs[1]), 10),
        txc_int_create_int_node(operand_strs[2], strlen(operand_strs[2]), 10),
        txc_int_create_int_node(operand_strs[3], strlen(operand_strs[3]), 10),
        txc_int_create_int_node(operand_strs[4], strlen(operand_strs[4]), 10)
    };
    for (size_t i = 0; i < 5; i++) {
        if (!txc_node_test_valid(op_nodes[i], true))
            return 2;
    }
    if (neg) {
        op_nodes[0] = txc_node_create_un_op(TXC_NEG, op_nodes[0]);
        op_nodes[2] = txc_node_create_un_op(TXC_NEG, op_nodes[2]);
        op_nodes[4] = txc_node_create_un_op(TXC_NEG, op_nodes[4]);
    }
    op_nodes[1] = txc_node_create_bin_op(type, op_nodes[0], op_nodes[1]);
    op_nodes[3] = txc_node_create_bin_op(type, op_nodes[3], op_nodes[4]);
    op_nodes[2] = txc_node_create_bin_op(type, op_nodes[2], op_nodes[3]);
    op_nodes[1] = txc_node_create_bin_op(type, op_nodes[1], op_nodes[2]);
    const txc_node *const result = txc_node_simplify(op_nodes[1]);
    if (!txc_node_test_valid(result, true))
        return 3;
    char *str = txc_node_to_str(result);
    txc_node_free(op_nodes[1]);
    txc_node_free(result);
    const bool true_neg = neg && strcmp(ZERO, sol_str) != 0;
    const char *const test_str = true_neg ? str + 2 : str;
    if (strncmp(test_str, sol_str, strlen(sol_str)) != 0) {
        fprintf(stderr, true_neg ? "Got:\n%s\nExpected:\n(-%s)\n" : "Got:\n%s\nExpected:\n%s\n", str, DIV_SOL);
        return 4;
    }
    free(str);
    return 0;
}

static void node_unsigned_add(void)
{
    const char *const operands_node[5] = { LONG_1, LONG_2, LONG_3, LONG_4, LONG_5 };
    exit(node_add_mul(UNSIGNED_ADD, operands_node, UNSIGNED_ADD_SOL));
}

static void node_signed_add(void)
{
    const char *const operands_node[5] = { LONG_1, LONG_2, LONG_3, LONG_4, LONG_5 };
    exit(node_add_mul(SIGNED_ADD, operands_node, SIGNED_ADD_SOL));
}

static void node_unsigned_mul(void)
{
    const char *const operands_node_1[5] = { LONG_1, LONG_2, LONG_3, LONG_4, LONG_5 };
    const int ec = node_add_mul(UNSIGNED_MUL, operands_node_1, MUL_SOL);
    if (ec != 0)
        exit(ec);
    const char *const operands_node_2[5] = { MUL_SOL, LONG_2, LONG_3, ZERO, LONG_5 };
    exit(node_add_mul(UNSIGNED_MUL, operands_node_2, ZERO));
}

static void node_signed_mul(void)
{
    const char *const operands_node_1[5] = { LONG_1, LONG_2, LONG_3, LONG_4, LONG_5 };
    const int ec = node_add_mul(SIGNED_MUL, operands_node_1, MUL_SOL);
    if (ec != 0)
        exit(ec);
    const char *const operands_node_2[5] = { MUL_SOL, LONG_2, LONG_3, ZERO, LONG_5 };
    exit(node_add_mul(SIGNED_MUL, operands_node_2, ZERO));
}

static int node_frac_int(bool inv)
{
    txc_node *const prod_node = txc_int_create_int_node(DEC_1, strlen(DEC_1), 10);
    txc_node *const div_node = txc_int_create_int_node(DIVISOR, strlen(DIVISOR), 10);
    if (!txc_node_test_valid(prod_node, true) || !txc_node_test_valid(div_node, true))
        return 1;
    txc_node *const frac = txc_node_create_bin_op(TXC_FRAC, inv ? prod_node : div_node, inv ? div_node : prod_node);
    if (!txc_node_test_valid(frac, true))
        return 2;
    txc_node *const result = txc_node_simplify(frac);
    char *const str = txc_node_to_str(result);
    txc_node_free(frac);
    txc_node_free(result);
    const char *const test_str = inv ? str + 9 : str;
    if (strncmp(test_str, DIV_SOL, strlen(DIV_SOL)) != 0) {
        fprintf(stderr, inv ? "Got:\n%s\nExpected:\n\\frac{1}{%s}\n" : "Got:\n%s\nExpected:\n%s\n", str, DIV_SOL);
        return 3;
    }
    free(str);
    return 0;
}

static void node_frac_int_normal(void)
{
    exit(node_frac_int(false));
}

static void node_frac_int_inverted(void)
{
    exit(node_frac_int(true));
}

static void parser_combined(void)
{
    parser("\\frac{0x80 \\cdot 0B10 - 200 + 5 \\cdot 5}{(8 - 2 - 2) \\cdot (5 - 8)} \\\\", "= (-\\frac{27}{4}) \\\\\n");
}

/* MAIN */

int main(int argc, char **argv)
{
    int ec;
    if (argc < 2) {
        fprintf(stderr, "No test name specified.\n");
        return -1;
    } else if (strcmp(argv[1], "test_start") == 0) {
        printf("Running tests for TeXalC %u.%u.%u:\n", TXC_VERSION_MAJOR, TXC_VERSION_MINOR, TXC_VERSION_PATCH);
        ec = -3;
    }
    TEST(test_test)
    TEST(integer_one)
    TEST(integer_zero)
    TEST(integer_copy)
    TEST(integer_create_bin)
    TEST(integer_create_dec)
    TEST(integer_create_hex)
    TEST(integer_unsigned_add)
    TEST(integer_signed_add)
    TEST(integer_unsigned_mul)
    TEST(integer_signed_mul)
    TEST(integer_gcd)
    TEST(integer_div_invalid)
    TEST(integer_div)
    TEST(integer_to_str)
    TEST(node_constants)
    TEST(node_create_nan)
    TEST(node_neg)
    TEST(node_unsigned_add)
    TEST(node_signed_add)
    TEST(node_unsigned_mul)
    TEST(node_signed_mul)
    TEST(node_frac_int_normal)
    TEST(node_frac_int_inverted)
    TEST(parser_combined)
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
