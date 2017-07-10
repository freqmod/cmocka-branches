/*
 * Copyright 2017 Nordic semiconductor.
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

/** Helper functions for wrapping defines below */
void _branch_test_wrapper(void **state);

/** Helper functions for wrapping defines below */
int _branch_teardown_wrapper(void **state);

/** Helper struct for wrapping defines below */
struct CMBUnitTestWrapper {
    CMUnitTestFunction test_func;
    CMFixtureFunction  teardown_func;
    void *initial_inner_state;
};

/** Initializes a CMUnitTest structure. 
  * This version of the function sets up the test to be used with branch_start & branch_end
  */
#define cmocka_unit_test_twigs(f) { #f, _branch_test_wrapper, NULL, _branch_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), NULL, NULL }))}

/** Initializes a CMUnitTest structure with a setup function. 
  *
  * This version of the function sets up the test to be used with branch_start & branch_end
  */
#define cmocka_unit_test_setup_twigs(f, setup) { #f, _branch_test_wrapper, setup, _branch_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), NULL, NULL }))}

/** Initializes a CMUnitTest structure with a teardown function. 
 *
 * This version of the function sets up the test to be used with branch_start & branch_end*/
#define cmocka_unit_test_teardown_twigs(f, teardown) { #f, _branch_test_wrapper, NULL, _branch_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), (teardown), NULL }))}

/**
 * Initialize an array of CMUnitTest structures with a setup function for a test
 * and a teardown function. Either setup or teardown can be NULL.
 *
 * This version of the function sets up the test to be used with branch_start & branch_end
 */
#define cmocka_unit_test_setup_teardown_twigs(f, setup, teardown) { #f, _branch_test_wrapper, setup, _branch_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), (teardown), NULL }))}

/**
 * Initialize a CMUnitTest structure with given initial state. It will be passed
 * to test function as an argument later. It can be used when test state does
 * not need special initialization or was initialized already.
 * @note If the group setup function initialized the state already, it won't be
 * overridden by the initial state defined here.
 *
 * This version of the function sets up the test to be used with branch_start & branch_end
 */
#define cmocka_unit_test_prestate_twigs(f, state) { #f, _branch_test_wrapper, NULL, _branch_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), NULL, NULL }))}

/**
 * Initialize a CMUnitTest structure with given initial state, setup and
 * teardown function. Any of these values can be NULL. Initial state is passed
 * later to setup function, or directly to test if none was given.
 * @note If the group setup function initialized the state already, it won't be
 * overridden by the initial state defined here.
 *
 * This version of the function sets up the test to be used with branch_start & branch_end
 */
#define cmocka_unit_test_prestate_setup_teardown_twigs(f, setup, teardown, state) { #f, _branch_test_wrapper, setup, _branch_teardown_wrapper, (void*) (&((const struct CMBUnitTestWrapper){ (f), (teardown), state }))}

/* API for use without the CMOCKA test runner */

/* Type used for inner functions that support branches. */
typedef void (*BranchInnerFunction)(void *state);


/**
 * Wrap a custom function for use with branches. This function should be used to wrap any function that calls branch_start and branch_end (when not using cmocka).
 * The wrapped function will be called repetitively until all branch combinations are executed.
 * This function cannot be used recursively.

 * @param State: Forwarded to the inner func - function.
 */

void _branch_custom_func_wrapper(BranchInnerFunction func, void *state);

#define branch_custom_func_wrapper(func, state) (_branch_custom_func_wrapper(func,state))

/*
 * This function prints the current path for branches that are executing.
   This function is primarily intended for error handling to report in which branch combination an error occurred.
 */
void _branch_print_current_path( void );

#define branch_print_current_path() (_branch_print_current_path());

/** @} */

#endif /* CMOCKA_BRANCHES_H_ */
