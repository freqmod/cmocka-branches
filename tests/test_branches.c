/*
 * Copyright 2017 Nordic Semiconductor <frederik.vestre@nordicsemi.no>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Use the unit test allocators */
#define UNIT_TESTING 1
#define TEST_FAILING 1
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <cmocka_branches.h>
#include <stdio.h>
static int runs;
static int total_runs;

static int branch_test_success_setup(void **state)
{
    (void)state;
    runs = 0;
    return 0;
}

static int branch_test_success_teardown(void **state)
{
    (void)state;
    assert_int_equal(runs, total_runs);
    return 0;
}

static void empty_test(void **state)
{
    (void)state;
}

static void no_branch_fail(void **state)
{
    (void)state;
    assert_int_equal(0,1);
}

static void branch_end_wo_start(void **state)
{
    (void)state;
    branch_end();
}

static void branch_end_wrong_name_start(void **state)
{
    (void)state;
    branch_start_count("Number match", 2, NULL);
    branch_end_named("ababa");
}

static void branch_end_wrong_number_start(void **state)
{
    (void)state;
    branch_start_count("", 1, NULL);
    branch_end_named("");
}

static void simple_branch_test_success(void **state)
{
    unsigned int aba_branch = branch_start_count("aba", 3, NULL);
    total_runs = 3; /* Initialize info for teardown */
    assert_int_equal(aba_branch, runs);
    branch_end_named("aba");
    runs++;
    (void)state;
}


static void multiple_branch_test_success(void **state)
{
    unsigned int aba_branch, baba_branch;
    const unsigned int branch_values[][2] = {{0,0}, {0,1}, {1,0}, {1,1}, {2,0},{2,1}};
    total_runs = sizeof(branch_values)/sizeof(branch_values[0]); /* Initialize info for teardown */

    aba_branch = branch_start_count("aba", 3, NULL);
    assert_int_equal(aba_branch, branch_values[runs][0]);
    branch_end_named("aba");
    baba_branch = branch_start_count("baba", 2, NULL);
    assert_int_equal(baba_branch, branch_values[runs][1]);
    branch_end_named("baba");
    runs++;
    (void)state;
}

static void simple_nested_branch_test_success(void **state)
{
    const unsigned int branch_values[][2] = {{0,0}, {0,1}, {1,0}, {1,1}, {2,0},{2,1}};
    unsigned int aba_branch, baba_branch;
    total_runs = sizeof(branch_values)/sizeof(branch_values[0]); /* Initialize info for teardown */

    aba_branch = branch_start_count("aba", 3, NULL);
    baba_branch = branch_start_count("bqba", 2, NULL);
    assert_int_equal(aba_branch, branch_values[runs][0]);
    assert_int_equal(baba_branch, branch_values[runs][1]);
    branch_end_named("bqba");
    branch_end_named("aba");
    runs++;
    (void)state;
}


static void varying_nested_branch_test_success(void **state)
{
    unsigned int branch_lvl_1, branch_lvl_2;
    const unsigned int branch_values[][2] = {{0,0}, {0,1}, {1,255}, {2,0}, {2,1},{2,2},{2,3}};
    total_runs = sizeof(branch_values)/sizeof(branch_values[0]); /* Initialize info for teardown */

    branch_lvl_1 = branch_start_count("aba", 3, NULL);
    branch_lvl_2 = 255;

    switch(branch_lvl_1)
    {
        case 0:
            branch_lvl_2 = branch_start_count("baba", 2, NULL);
            branch_end_named("baba");
            break;
        case 2:
            branch_lvl_2 = branch_start_count("caba", 4, NULL);
            branch_end_named("caba");
            break;
        break;
    }
    branch_end_named("aba");
    assert_int_equal(branch_lvl_1, branch_values[runs][0]);
    assert_int_equal(branch_lvl_2, branch_values[runs][1]);
    runs++;
    (void)state;
}

static void varying_double_nested_branch_test_success(void **state)
{
    unsigned int branch_lvl_1, branch_lvl_2, branch_lvl_3;
    const unsigned int branch_values[][3] = {{0,0,255}, {0,1,255}, {1,255, 255},
                                           {2,0,0}, {2,0,1},{2,0,2},{2,0,3},
                                           {2,1,255}, {2,2,255}};
    total_runs = sizeof(branch_values)/sizeof(branch_values[0]); /* Initialize info for teardown */

    branch_lvl_1 = branch_start_count("first_level", 3, NULL);
    branch_lvl_2 = 255;
    branch_lvl_3 = 255;

    switch(branch_lvl_1)
    {
        case 0:
            branch_lvl_2 = branch_start_count("second_level_first_subbranch", 2, NULL);
            branch_end_named("second_level_first_subbranch");
            break;
        case 2:
            branch_lvl_2 = branch_start_count("second_level_third_subbranch", 3, NULL);
            if(branch_lvl_2 == 0) {
                    branch_lvl_3 = branch_start_count("third_level_first_subbranch", 4, NULL);
                    branch_end_named("third_level_first_subbranch");
            }
            branch_end_named("second_level_third_subbranch");
            break;
    }
    branch_end_named("first_level");

    assert_int_equal(branch_lvl_1, branch_values[runs][0]);
    assert_int_equal(branch_lvl_2, branch_values[runs][1]);
    assert_int_equal(branch_lvl_3, branch_values[runs][2]);
    runs++;
    (void)state;
}

static void varying_sequential_nested_branch_test_success(void **state)
{
    unsigned int branch_lvl_1, branch_lvl_2, branch_lvl_3, branch_lvl_4, branch_lvl_5, branch_lvl_6;
    const unsigned int branch_values[][6] = {
                                           {0,0,255,255, 255, 255},
                                           {0,1,255,255, 255, 255},
                                           {1,255,255,255, 255, 255},
                                           {2,0,0,255, 0, 0},
                                           {2,0,0,255, 1, 0},
                                           {2,0,1,255, 0, 0},
                                           {2,0,1,255, 1, 0},
                                           {2,0,2,255, 0, 0},
                                           {2,0,2,255, 1, 0},
                                           {2,0,2,255, 1, 1},
                                           {2,1,0,0, 0, 0},
                                           {2,1,0,0, 1, 0},
                                           {2,1,0,1, 0, 0},
                                           {2,1,0,1, 1, 0},
                                           {2,1,1,0, 0, 0},
                                           {2,1,1,0, 1, 0},
                                           {2,1,1,1, 0, 0},
                                           {2,1,1,1, 1, 0},
                                           {2,1,2,0, 0, 0},
                                           {2,1,2,0, 1, 0},
                                           {2,1,2,1, 0, 0},
                                           {2,1,2,1, 1, 0},
                                           {2,1,2,1, 1, 1},
                                           {2,2,255,255, 0, 0},
                                           {2,2,255,255, 1, 0},
                                           {2,2,255,255, 1, 1}};
    total_runs = sizeof(branch_values)/sizeof(branch_values[0]); /* Initialize info for teardown */

    branch_lvl_1 = branch_start_count("first_level", 3, NULL);
    branch_lvl_2 = 255;
    branch_lvl_3 = 255;
    branch_lvl_4 = 255;
    branch_lvl_5 = 255;
    branch_lvl_6 = 255;

    switch(branch_lvl_1)
    {
        case 0:
            branch_lvl_2 = branch_start_count("second_level_first_subbranch", 2, NULL);
            branch_end_named("second_level_first_subbranch");
            break;
        case 2:
            branch_lvl_2 = branch_start_count("second_level_third_subbranch", 3, NULL);
#if 1
            if(branch_lvl_2 == 0 || branch_lvl_2 == 1) {
                    branch_lvl_3 = branch_start_count("third_level_first_subbranch", 3, NULL);
                    branch_end_named("third_level_first_subbranch");
            }
#endif
#if 1
            if(branch_lvl_2==1)
            {
                branch_lvl_4 = branch_start_count("third_level_second_subbranch", 2, NULL);
                branch_end_named("third_level_second_subbranch");
            }
#endif
            /* It seems like this (later subbranch) over shadows all earlier subbranchs on this level */
            branch_lvl_5 = branch_start_count("third_level_third_subbranch", 2, NULL);
            branch_end_named("third_level_third_subbranch");

            branch_end_named("second_level_third_subbranch");


#if 1
            branch_lvl_6 = branch_start_count("second_level_forth_subbranch", 2, NULL);
            branch_end_named("second_level_forth_subbranch");
#endif
            break;
    }

    branch_end_named("first_level");
    assert_int_equal(branch_lvl_1, branch_values[runs][0]);
    assert_int_equal(branch_lvl_2, branch_values[runs][1]);
    assert_int_equal(branch_lvl_3, branch_values[runs][2]);
    assert_int_equal(branch_lvl_4, branch_values[runs][3]);
    assert_int_equal(branch_lvl_5, branch_values[runs][4]);
    assert_int_equal(branch_lvl_6, branch_values[runs][5]);
    runs++;
    (void)state;
}

/* Example/demo unittest from documentation, does not actually assert anything*/
static void phy_change_test(void **state)
{
    static char const * const  ch_rsp_names[] = {"change_to_2mbps", "change to coded", "no change"};
    static char const * const  ch_rsp_error_code_2m[]  = {"success",
                                                          "destination phy not supported"};
    static char const * const  ch_rsp_error_code_cd[]  = {"success",
                                                          "destination phy not supported",
                                                          "not enough time",
                                                          "<not entered>"};
    static char const * const  ch_rsp_error_code_dle[] = {"larger packets",
                                                          "smaller packets",
                                                          "same size packets",
                                                          "no packet size change",
                                                          "<not entered>"};
    unsigned int ch_rsp_phy, ch_rsp_errcode, dle_rsp_errcode;
    ch_rsp_phy = branch_start_count("change_response_phy", 3, ch_rsp_names);
    /* Set default values here for use by print later */
    ch_rsp_errcode = 3;
    dle_rsp_errcode = 4;
    switch(ch_rsp_phy)
    {
        case 0: /* change_to_2mbps */
            ch_rsp_errcode = branch_start_count("ch_rsp_error_code", 2, ch_rsp_error_code_2m);
            /* Do some validation here */
            branch_end_named("ch_rsp_error_code");
            dle_rsp_errcode = branch_start_count("data length procedure response error code", 4, ch_rsp_error_code_dle);
            /* Do some other validation here */
            branch_end_named("data length procedure response error code");
            break;
        case 1: /* change_to_coded */
            ch_rsp_errcode = branch_start_count("ch_rsp_error_code_cd", 3, ch_rsp_error_code_cd);
            /* Do some validation here */
            branch_end_named("ch_rsp_error_code_cd");
            break;
        case 2: /* no_change */
            ch_rsp_errcode = branch_start_count("ch_rsp_error_code", 2, NULL);
            /* Do some validation here */
            branch_end_named("ch_rsp_error_code");
            break;
    }
    branch_end_named("change_response_phy");
    printf("Phy change test: Phy: %s (%u),\tCode: %s (%u), \tDle: %s (%u)\n",
            ch_rsp_names[ch_rsp_phy], ch_rsp_phy,
            ch_rsp_error_code_cd[ch_rsp_errcode], ch_rsp_errcode,
            ch_rsp_error_code_dle[dle_rsp_errcode], dle_rsp_errcode);
    (void)state;
}

static void branch_test_errname(void **state) {
    static char const * const  aba_names[] = {"aCase0", "aCase1"};
    static char const * const  caca_names[] = {"dCase0", "dCase1", "dCase2"};

    switch(branch_start_count("aba", 2, aba_names)) {
        case 0:
            printf("---_--aCase0\n");
            break;
        case 1:
            switch(branch_start_count("daca", 3, caca_names)) {
                case 0:
                    printf("-----_dCase0\n");
                    assert_int_equal(42, 42);
                    break;
                case 1:
                    printf("-----_dCase1\n");
                    assert_int_equal(43, 44);
                    printf("-----_dCase1--\n");
                    break;
                case 2:
                    printf("-----_dCase2\n");
                    assert_int_equal(43, 43);
                    break;
            }
            branch_end_named("daca");
            break;
    }
    branch_end_named("aba");
    printf("aCaseEnd\n");
    (void)state;
}
static void mistmatched_branch_start(void **state) {
    branch_start_count("aba", 2, NULL);
    (void)state;
}

static int branch_test_fail_setup(void **state)
{
    (void)state;
    branch_start_count("eba", 2, NULL);
    return 0;
}

static int branch_test_fail_teardown(void **state)
{
    (void)state;
    branch_end_named("eba");
    return 0;
}



int main(void) {
    const struct CMUnitTest test_group1[] = {
        cmocka_unit_test_setup_teardown_twigs(simple_branch_test_success, branch_test_success_setup, branch_test_success_teardown),
        cmocka_unit_test_setup_teardown_twigs(multiple_branch_test_success, branch_test_success_setup, branch_test_success_teardown),
        cmocka_unit_test_setup_teardown_twigs(simple_nested_branch_test_success, branch_test_success_setup, branch_test_success_teardown),
        cmocka_unit_test_setup_teardown_twigs(varying_nested_branch_test_success, branch_test_success_setup, branch_test_success_teardown),
        cmocka_unit_test_setup_teardown_twigs(varying_double_nested_branch_test_success, branch_test_success_setup, branch_test_success_teardown),
        cmocka_unit_test_setup_teardown_twigs(varying_sequential_nested_branch_test_success, branch_test_success_setup, branch_test_success_teardown),
        cmocka_unit_test_twigs(phy_change_test),
    };

    const struct CMUnitTest test_group_fail_expected[] = {
        cmocka_unit_test_twigs(no_branch_fail),
        cmocka_unit_test_twigs(branch_test_errname),
        cmocka_unit_test_twigs(branch_end_wo_start),
        cmocka_unit_test_twigs(branch_end_wrong_name_start),
        cmocka_unit_test_twigs(branch_end_wrong_number_start),
        cmocka_unit_test_twigs(mistmatched_branch_start),
        cmocka_unit_test_setup_teardown_twigs(empty_test, branch_test_fail_setup, branch_test_fail_teardown),
        cmocka_unit_test_setup_twigs(empty_test, branch_test_fail_setup),
        cmocka_unit_test_teardown_twigs(empty_test, branch_test_fail_teardown),
    };

    int result = 0;
    result += cmocka_run_group_tests(test_group1, NULL, NULL);
#ifdef TEST_FAILING
    result += cmocka_run_group_tests(test_group_fail_expected, NULL, NULL);
#else
    (void) test_group_fail_expected;
#endif

    return result;
}
