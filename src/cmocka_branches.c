/*
 * Copyright 2008 Google Inc.
 * Copyright 2014-2015 Andreas Schneider <asn@cryptomilk.org>
 * Copyright 2015      Jakub Hrozek <jakub.hrozek@posteo.se>
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <stdint.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cmocka_branches.h"

#if defined(HAVE_GCC_THREAD_LOCAL_STORAGE)
# define CMOCKA_THREAD __thread
#elif defined(HAVE_MSVC_THREAD_LOCAL_STORAGE)
# define CMOCKA_THREAD __declspec(thread)
#else
# define CMOCKA_THREAD
#endif


/* CMOCKA utils (copied from cmocka.c) */

/* Printf formatting for source code locations. */
#define SOURCE_LOCATION_FORMAT "%s:%u"

//void cm_print_error(const char * const format, ...) CMOCKA_PRINTF_ATTRIBUTE(1, 2);

#define branch_print_error print_error
#define cm_print_error print_error

/* Doubly linked list node. */
typedef struct ListNode {
    void *value;
    int refcount;
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

/* Used by list_free() to deallocate values referenced by list nodes. */
typedef void (*CleanupListValue)(const void *value, void *cleanup_value_data);


static ListNode* list_initialize(ListNode * const node);
static ListNode* list_add(ListNode * const head, ListNode *new_node);
static ListNode* list_add_value(ListNode * const head, void *value,
                                     const int count);
static ListNode* list_remove(
    ListNode * const node, const CleanupListValue cleanup_value,
    void * const cleanup_value_data);
static void list_remove_free(
    ListNode * const node, const CleanupListValue cleanup_value,
    void * const cleanup_value_data);
static int list_empty(const ListNode * const head);
static int list_first(ListNode * const head, ListNode **output);
static ListNode* list_free(
    ListNode * const head, const CleanupListValue cleanup_value,
    void * const cleanup_value_data);

/* Initialize a list node. */
static ListNode* list_initialize(ListNode * const node) {
    node->value = NULL;
    node->next = node;
    node->prev = node;
    node->refcount = 1;
    return node;
}


/*
 * Adds a value at the tail of a given list.
 * The node referencing the value is allocated from the heap.
 */
static ListNode* list_add_value(ListNode * const head, void *value,
                                     const int refcount) {
    ListNode * const new_node = (ListNode*)malloc(sizeof(ListNode));
    assert_non_null(head);
    assert_non_null(value);
    new_node->value = value;
    new_node->refcount = refcount;
    return list_add(head, new_node);
}


/* Add new_node to the end of the list. */
static ListNode* list_add(ListNode * const head, ListNode *new_node) {
    assert_non_null(head);
    assert_non_null(new_node);
    new_node->next = head;
    new_node->prev = head->prev;
    head->prev->next = new_node;
    head->prev = new_node;
    return new_node;
}


/* Remove a node from a list. */
static ListNode* list_remove(
        ListNode * const node, const CleanupListValue cleanup_value,
        void * const cleanup_value_data) {
    assert_non_null(node);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    if (cleanup_value) {
        cleanup_value(node->value, cleanup_value_data);
    }
    return node;
}


/* Remove a list node from a list and free the node. */
static void list_remove_free(
        ListNode * const node, const CleanupListValue cleanup_value,
        void * const cleanup_value_data) {
    assert_non_null(node);
    free(list_remove(node, cleanup_value, cleanup_value_data));
}


/*
 * Frees memory kept by a linked list The cleanup_value function is called for
 * every "value" field of nodes in the list, except for the head.  In addition
 * to each list value, cleanup_value_data is passed to each call to
 * cleanup_value.  The head of the list is not deallocated.
 */
static ListNode* list_free(
        ListNode * const head, const CleanupListValue cleanup_value,
        void * const cleanup_value_data) {
    assert_non_null(head);
    while (!list_empty(head)) {
        list_remove_free(head->next, cleanup_value, cleanup_value_data);
    }
    return head;
}


/* Determine whether a list is empty. */
static int list_empty(const ListNode * const head) {
    assert_non_null(head);
    return head->next == head;
}


/* Returns the first node of a list */
static int list_first(ListNode * const head, ListNode **output) {
    ListNode *target_node;
    assert_non_null(head);
    if (list_empty(head)) {
        return 0;
    }
    target_node = head->next;
    *output = target_node;
    return 1;
}


/* Deallocate a value referenced by a list. */
static void free_value(const void *value, void *cleanup_value_data) {
    (void)cleanup_value_data;
    assert_non_null(value);
    free((void*)value);
}

/*****************************************************************************/
/**** Branch related code                                                    ***/
/*****************************************************************************/


/* Branch related information data types */
typedef enum
{
    FORK_BRANCH_STATE_UNINITIALIZED = 0,  /* This twig has never been executed for any sub combinations */
    FORK_BRANCH_STATE_DISCOVERED = 1,     /* This twig has never been executed for at least one sub combination */
} BranchTwigState;

typedef enum
{
    FORK_RESTART_CODE_COMPLETE = 0,
    FORK_RESTART_CODE_RESTART = 1,
    FORK_RESTART_CODE_ERROR = 2,
} BranchRestartCode;

struct BranchInformation_s;
typedef struct
{
    unsigned int value;

    /* Bookkeeping info */
    BranchTwigState state;
    ListNode* current_prev_subbranch;
    struct BranchInformation_s* parent_branch;
    ListNode subbranches;
} BranchTwig;


typedef struct BranchInformation_s
{
    /* User provided branch info*/
    char const *name;
    char const *file;
    char const *function_name;
    unsigned int line;
    unsigned int num_twigs;
    char const * const * twig_names;

    /* Bookkeeping info */
    BranchTwig *parent_twig;
    BranchTwig *twigs;
    unsigned int current_twig_idx;

} BranchInformation;

/* Global collection of branch related information */
typedef struct
{
    BranchInformation *current_branch;
    BranchTwig *current_twig;
    BranchInformation *next_mutate_subbranch;
    BranchInformation *prev_mutate_subbranch;
    unsigned int next_mutate_subbranch_nesting_level;
    unsigned int prev_mutate_subbranch_nesting_level;
    unsigned int nesting_level;
    ListNode branchlines;
    BranchTwig trunk;
} BranchesInformation;

/* Struct containing all global state related to test branches */
static CMOCKA_THREAD BranchesInformation global_branch_information;
static CMOCKA_THREAD int global_branches_enabled = 0;

/* Functions */

static int branch_info_equal(BranchInformation const * const subbranch_information, const char* const name, const unsigned int num_twigs, const char* const file, const unsigned int line, const char* const function_name)
{
    return (subbranch_information->name == name &&
            subbranch_information->num_twigs == num_twigs &&
            strcmp(subbranch_information->file, file) == 0 &&
            subbranch_information->line == line &&
            strcmp(subbranch_information->function_name, function_name) == 0);
}

static void branch_try_mutate( void )
{
    if( global_branch_information.prev_mutate_subbranch == global_branch_information.current_branch)
    {
        /* Mutate */
        global_branch_information.current_branch->current_twig_idx++;
        global_branch_information.prev_mutate_subbranch = NULL; /* Record that we have mutated a subbranch */
    }
    else if((global_branch_information.nesting_level > global_branch_information.prev_mutate_subbranch_nesting_level) ||
             (((global_branch_information.nesting_level + 1) == global_branch_information.prev_mutate_subbranch_nesting_level) && (global_branch_information.prev_mutate_subbranch == NULL)))
    {
        global_branch_information.current_branch->current_twig_idx = 0;
    }
}

unsigned int _branch_start(const char* const name, unsigned int num_twigs, char const * const * const twig_names, const char* const file, const int line, const char* const function_name)
{
    int branch_ret_val = 0;
    if(num_twigs < 2) {
        cm_print_error(SOURCE_LOCATION_FORMAT
                       ": error: Branch start in function %s requested for %d branches, only 2 or more branches are supported\n",
                       file, line,
                       function_name, num_twigs);
        _fail(file, line);
        return 0;
    }

    if(!global_branches_enabled) {
        cm_print_error(SOURCE_LOCATION_FORMAT
                       ": error: Branch start in function %s with name %s called outside a test.\n",
                       file, line,function_name, name);
        _fail(file, line);
        return 0;
    }

    switch (global_branch_information.current_twig->state) {
        case FORK_BRANCH_STATE_UNINITIALIZED:
        {
            unsigned int i;
            BranchInformation * const new_branch_information =
                    (BranchInformation*)malloc(sizeof(BranchInformation));
            list_add_value(&global_branch_information.current_twig->subbranches, new_branch_information, 0);
            /* Initialize branch struct */
            new_branch_information->name = name;
            new_branch_information->file = file;
            new_branch_information->function_name = function_name;
            new_branch_information->line = line;
            new_branch_information->num_twigs = num_twigs;
            new_branch_information->parent_twig = global_branch_information.current_twig;
            new_branch_information->twig_names = twig_names;
            new_branch_information->twigs = (BranchTwig*)malloc(sizeof(BranchTwig)*new_branch_information->num_twigs);
            new_branch_information->current_twig_idx = 0;
            /* Add the twigs */
            for(i = 0; i < num_twigs; i++) {
                new_branch_information->twigs[i].state = FORK_BRANCH_STATE_UNINITIALIZED;
                new_branch_information->twigs[i].current_prev_subbranch = &new_branch_information->twigs[i].subbranches;
                new_branch_information->twigs[i].parent_branch = new_branch_information;
                new_branch_information->twigs[i].value = i;
                list_initialize(&new_branch_information->twigs[i].subbranches);
            }
            /* Update sub branch information for the current branch level */
            global_branch_information.current_twig->current_prev_subbranch = (global_branch_information.current_twig->current_prev_subbranch->next);
            global_branch_information.current_branch = new_branch_information;
            assert_ptr_equal(global_branch_information.current_twig->current_prev_subbranch->value, new_branch_information);

            /* Set up the the newly created branch (in next nesting level) as the current twig */
            global_branch_information.current_twig = &global_branch_information.current_branch->twigs[new_branch_information->current_twig_idx];
            global_branch_information.nesting_level++;

        }
        break;
        case FORK_BRANCH_STATE_DISCOVERED:
        {
            ListNode * subbranch_node;
            BranchInformation const * subbranch_information;
            if (global_branch_information.current_twig->current_prev_subbranch->next == &global_branch_information.current_twig->subbranches) {
                /* We have looped around the list and now have more sub branches this time than previous runs */
                cm_print_error("Failend %s\n", global_branch_information.current_twig->current_prev_subbranch->value ? ((const BranchInformation*)global_branch_information.current_twig->current_prev_subbranch->value)->name : "<null>");
                _fail(file, line);
            } else {
                /* Change to the next sub branch of this twig */
                global_branch_information.current_twig->current_prev_subbranch = global_branch_information.current_twig->current_prev_subbranch->next;
                global_branch_information.current_branch = (BranchInformation*) global_branch_information.current_twig->current_prev_subbranch->value;
            }

            /* Validate that the branch matches the parameters */
            assert_ptr_equal(global_branch_information.current_twig, ((BranchInformation*)global_branch_information.current_twig->current_prev_subbranch->value)->parent_twig);
            subbranch_node = global_branch_information.current_twig->current_prev_subbranch;
            subbranch_information = (const BranchInformation*) subbranch_node->value;
            assert_non_null(subbranch_information);

            if(!branch_info_equal(subbranch_information, name, num_twigs, file, line, function_name)) {
                cm_print_error("Failinfo\n");
                _fail(file, line);
            }

            /* Update global pointers */
            global_branch_information.current_branch = (BranchInformation*) subbranch_information;

            branch_try_mutate();

            /* Update global pointers */
            global_branch_information.current_twig = &global_branch_information.current_branch->twigs[global_branch_information.current_branch->current_twig_idx];
            global_branch_information.nesting_level++;

            /* Update return value */
            branch_ret_val = global_branch_information.current_twig->value;
        }
        break;
        default:
            cm_print_error("Fail state in branch_end\n");
            _fail(file, line);
        break;
    }
    return branch_ret_val;
}

void _branch_end(const char* const name, const char* const file, const int line, const char* const function_name)
{
    BranchTwig *inner_twig;
    if(!global_branches_enabled) {
        cm_print_error(SOURCE_LOCATION_FORMAT
                       ": error: Branch start in function %s called outside a test.\n",
                       file, line,function_name);
        _fail(file, line);
        return;
    }
    if(global_branch_information.current_branch == NULL) {
        cm_print_error(SOURCE_LOCATION_FORMAT
                       ": error: Branch end requested in function %s using name \"%s\", but no branch started.\n",
                       file, line,
                       function_name, name);
        _fail(file, line);
        return;
    }
    if(strcmp(name, global_branch_information.current_branch->name) != 0) {
        cm_print_error(SOURCE_LOCATION_FORMAT
                       ": error: Branch end in function %s using name \"%s\". Expected name \"%s\" as used by last branch start\n",
                       file, line,
                       function_name, name, global_branch_information.current_branch->name);
        _fail(file, line);
        return;
    }

    if(global_branch_information.current_branch != global_branch_information.current_branch->parent_twig->current_prev_subbranch->value) {
        cm_print_error(SOURCE_LOCATION_FORMAT
                       ": error: Inconsistent amount of branch start/end function pairs detected in function %s using name \"%s\", with cmocka recorded branch name \"%s\".\n",
                       file, line,
                       function_name, name, global_branch_information.current_branch->name);
        _fail(file, line);
        return;
    }

    if(global_branch_information.current_twig->state == FORK_BRANCH_STATE_UNINITIALIZED) {
        global_branch_information.current_twig->state = FORK_BRANCH_STATE_DISCOVERED;
    }
    else if (global_branch_information.current_twig->state != FORK_BRANCH_STATE_DISCOVERED) {
        _fail(file, line);
    }

    if((global_branch_information.current_branch->current_twig_idx != (global_branch_information.current_branch->num_twigs - 1)) &&
       (global_branch_information.next_mutate_subbranch_nesting_level <= global_branch_information.nesting_level)) {
            /* Mark this subbranch as pending mutation */
            global_branch_information.next_mutate_subbranch = global_branch_information.current_branch;
            global_branch_information.next_mutate_subbranch_nesting_level = global_branch_information.nesting_level;
    }

    /* Un-nest branch level */
    global_branch_information.current_twig->current_prev_subbranch = &global_branch_information.current_twig->subbranches; /* Reset twig of the (inner) subbranch we are leaving */
    inner_twig = global_branch_information.current_twig;
    global_branch_information.current_twig = global_branch_information.current_branch->parent_twig;
    global_branch_information.current_branch =  inner_twig->parent_branch->parent_twig->parent_branch;
    global_branch_information.nesting_level--;

}

static BranchRestartCode branches_restart( void )
{
    if(((global_branch_information.current_twig != &global_branch_information.trunk) ||
                   (global_branch_information.nesting_level != 0))) {
        cm_print_error("ERROR: Number of branch ends doesn't match branch starts in top level\n");
        fail();
        return FORK_RESTART_CODE_ERROR;
    }
    if((global_branch_information.current_twig->current_prev_subbranch->next != &global_branch_information.current_twig->subbranches)) {
        cm_print_error("ERROR: Number of branches in top level not consistent between runs\n");
        fail();
        return FORK_RESTART_CODE_ERROR;
    }

    if(global_branch_information.current_twig->state == FORK_BRANCH_STATE_UNINITIALIZED) {
        global_branch_information.current_twig->state = FORK_BRANCH_STATE_DISCOVERED;
    }

    global_branch_information.prev_mutate_subbranch = global_branch_information.next_mutate_subbranch;
    global_branch_information.prev_mutate_subbranch_nesting_level = global_branch_information.next_mutate_subbranch_nesting_level;
    global_branch_information.next_mutate_subbranch = NULL;
    global_branch_information.next_mutate_subbranch_nesting_level = 0;

    /* Move to the start of the sub branch list for the top twig */
    global_branch_information.current_twig->current_prev_subbranch = &global_branch_information.current_twig->subbranches; /* Before first subbranch in list */
    global_branch_information.current_branch = (BranchInformation*) global_branch_information.current_twig->current_prev_subbranch->value;

    return global_branch_information.prev_mutate_subbranch != NULL ? FORK_RESTART_CODE_RESTART : FORK_RESTART_CODE_COMPLETE;
}

static void branches_init( void )
{
    global_branch_information.trunk.state = FORK_BRANCH_STATE_UNINITIALIZED;
    global_branch_information.trunk.parent_branch = NULL;
    list_initialize(&global_branch_information.trunk.subbranches);
    list_initialize(&global_branch_information.branchlines);

    global_branch_information.current_branch = NULL;
    global_branch_information.trunk.current_prev_subbranch = &global_branch_information.trunk.subbranches;
    global_branch_information.current_twig = &global_branch_information.trunk;
    global_branch_information.next_mutate_subbranch = NULL;
    global_branch_information.prev_mutate_subbranch = NULL;
    global_branch_information.nesting_level = 0;
    global_branch_information.next_mutate_subbranch_nesting_level = 0;
    global_branches_enabled = 1;
}

static void free_branch(const void *value, void *cleanup_value_data);

static void free_branch_twig(BranchTwig *twig, void *cleanup_value_data)
{
    if(twig != NULL) {
        list_free(&twig->subbranches, free_branch, cleanup_value_data);
    }
}

static void free_branch(const void *value, void *cleanup_value_data)
{
    unsigned int i;
    if(value != NULL) {
        BranchInformation * const info = (BranchInformation * ) value;
        for(i = 0; i < info->num_twigs; i++) {
            free_branch_twig(&info->twigs[i], cleanup_value_data);
        }
        free(info->twigs);
        free(info);
    }
}

static void branch_print_twig_name(const BranchTwig *twig, unsigned int nesting)
{
    unsigned int i;
    for(i = 0;i < nesting;i++) {
        branch_print_error("  ");
    }

    if(twig) {
        if(twig->parent_branch->twig_names != NULL) {
            branch_print_error("- %s (%s, %d)\n", twig->parent_branch->name, twig->parent_branch->twig_names[twig->value], twig->value);
        }
        else {
            branch_print_error("- %s (%d)\n", twig->parent_branch->name, twig->value);
        }
    } else
    {
        branch_print_error("- ????\n");
    }
}

static void branch_print_current_path( void )
{
    unsigned int nesting;
    ListNode branch_path;
    ListNode* current_path_element;
    BranchTwig *current_twig = global_branch_information.current_twig;
    list_initialize(&branch_path);
    while(current_twig->parent_branch != NULL) {
        list_add_value(&branch_path, current_twig, 0);
        current_twig = current_twig->parent_branch->parent_twig;
    }
    current_path_element = branch_path.prev;
    branch_print_error("\n");
    nesting = 0;
    while(current_path_element != &branch_path) {
        ListNode *current_branch_node;
        current_twig = (BranchTwig*)current_path_element->value;
        current_branch_node = current_twig->parent_branch->parent_twig->subbranches.next;

        while(current_branch_node->value != current_twig->parent_branch) {
            BranchInformation *branch_info = (BranchInformation*)current_branch_node->value;
            branch_print_twig_name(&branch_info->twigs[branch_info->current_twig_idx], nesting);
            current_branch_node = current_branch_node->next;
        }
        branch_print_twig_name(current_twig, nesting);
        current_path_element = current_path_element->prev;
        nesting++;
    }
    list_free(&branch_path, NULL, NULL);
}

static void branch_post_cleanup(void )
{
    list_free(&global_branch_information.trunk.subbranches, free_branch, (void*)0);
    global_branch_information.current_branch = NULL;
    global_branches_enabled = 0;
}

void _branch_test_wrapper(void **state)
{
    unsigned int branch_restart_code;
    struct CMBUnitTestWrapper *wrap_state = (struct CMBUnitTestWrapper*)*state;
    /* wrap_state is const, so we put the void here in a stack variable in case the test tries to assign to it */
    void *initial_state = wrap_state->initial_inner_state;

    branches_init();
    do {
        wrap_state->test_func(&initial_state);
    } while((branch_restart_code = branches_restart()) == FORK_RESTART_CODE_RESTART);
    branch_post_cleanup();
}

int _branch_teardown_wrapper(void **state)
{
    unsigned int rc = 0;
    struct CMBUnitTestWrapper *wrap_state = (struct CMBUnitTestWrapper*)*state;
    /* wrap_state is const, so we put the void here in a stack variable in case the test tries to assign to it */
    void *initial_state = wrap_state->initial_inner_state;

    /* If global_branches_enabled we did not exit cleanly, print the current branch for tracing errors */
    if(global_branches_enabled) {
        branch_print_error("Branch path: ");
        branch_print_current_path();
        branch_post_cleanup();
        return 0;
    }

    if(wrap_state->teardown_func != NULL) {
        rc = wrap_state->teardown_func(&initial_state);
    }

    return rc;
}
