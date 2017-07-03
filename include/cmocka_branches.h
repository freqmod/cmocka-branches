/*
 * Copyright 2008 Google Inc.
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
#ifndef CMOCKA_BRANCHES_H_
#define CMOCKA_BRANCHES_H_

#include "cmocka.h"

#ifndef DOXYGEN
unsigned int _branch_start(const char* const name, unsigned int num_twigs, char const * const * const twig_names, const char* const file, const int line, const char* const function_name);
#endif

#ifdef DOXYGEN
/**
 * @brief Create an n-way for point in the test.
 *
 *
 * The test case will be ran repeatedly with every combinations for branch points.
 *
 * @param[in]  name   A name for the branch point, to be reported in error messages.
 *                    If the name is not an empty string it is validated to match at branch_end.

 * @param[in]  num_branchs  The number of branchs to branch into.
 *
 *
 * @returns An integer 0 < num_branchs, depending on the test repetition.
 */
void branch_start_count(const char *name, unsigned int num_branchs, char const * const * const twig_names);
#else
#define branch_start_count(name, num_branchs, twig_names) \
    _branch_start(name, num_branchs, twig_names, __FILE__, __LINE__, __func__)
#endif

#ifdef DOXYGEN
/**
 * @brief Create a 2-way for point in the test with an empty name
 *
 * @see branch_start_count for more info
 */
void branch_start();
#else
#define branch_start() \
    _branch_start("", 2, NULL,  __FILE__, __LINE__, __func__)
#endif

#ifdef DOXYGEN
/**
 * @brief End an n-way for point in the test.
 *
 * @param[in]  name   A name for the branch point, to be reported in error messages
                      This must match the name provided in branch_start.
 *
 * @see branch_start_count for more info
 */
void branch_end_named(const char *name);
#else
#define branch_end_named(name) \
    _branch_end(name, __FILE__, __LINE__, __func__)
#endif

#ifdef DOXYGEN
/**
 * @brief End an n-way for point in the test using an empty name.
 *
 * @see branch_start_count for more info
 */
void branch_end();
#else
void _branch_end(const char* const name, const char* const file, const int line, const char* const function_name);
#define branch_end() \
    _branch_end("", __FILE__, __LINE__, __func__)
#endif


enum CMUnitTestStatus {
    CM_TEST_NOT_STARTED,
    CM_TEST_PASSED,
    CM_TEST_FAILED,
    CM_TEST_ERROR,
    CM_TEST_SKIPPED,
};

void twigs_test_wrapper(void **state);

int twigs_teardown_wrapper(void **state);

struct CMBUnitTestWrapper {
    CMUnitTestFunction test_func;
    CMFixtureFunction  teardown_func;
    void *initial_inner_state;
};


/** Initializes a CMUnitTest structure. */
#define cmocka_unit_test_twigs(f) { #f, twigs_test_wrapper, NULL, twigs_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), NULL, NULL }))}

/** Initializes a CMUnitTest structure with a setup function. */
#define cmocka_unit_test_setup_twigs(f, setup) { #f, twigs_test_wrapper, setup, twigs_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), NULL, NULL }))}

/** Initializes a CMUnitTest structure with a teardown function. */
#define cmocka_unit_test_teardown_twigs(f, teardown) { #f, twigs_test_wrapper, NULL, twigs_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), (teardown), NULL }))}

/**
 * Initialize an array of CMUnitTest structures with a setup function for a test
 * and a teardown function. Either setup or teardown can be NULL.
 */
#define cmocka_unit_test_setup_teardown_twigs(f, setup, teardown) { #f, twigs_test_wrapper, setup, twigs_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), (teardown), NULL }))}

/**
 * Initialize a CMUnitTest structure with given initial state. It will be passed
 * to test function as an argument later. It can be used when test state does
 * not need special initialization or was initialized already.
 * @note If the group setup function initialized the state already, it won't be
 * overridden by the initial state defined here.
 */
#define cmocka_unit_test_prestate_twigs(f, state) { #f, twigs_test_wrapper, NULL, twigs_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), NULL, NULL }))}

/**
 * Initialize a CMUnitTest structure with given initial state, setup and
 * teardown function. Any of these values can be NULL. Initial state is passed
 * later to setup function, or directly to test if none was given.
 * @note If the group setup function initialized the state already, it won't be
 * overridden by the initial state defined here.
 */
#define cmocka_unit_test_prestate_setup_teardown_twigs(f, setup, teardown, state) { #f, twigs_test_wrapper, setup, twigs_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), (teardown), state }))}


/** @} */

#endif /* CMOCKA_BRANCHES_H_ */
