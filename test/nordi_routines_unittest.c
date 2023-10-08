#include "nordi_routines_unittest.h"
#include <stdbool.h>

static void
dummy_callback_null(void* args) {
    assert_null(args);
}

struct dummy_struct {
    int sample;
};

#define MOCKED_SAMPLE 42
#define MOCKED_STRUCT ((struct dummy_struct){.sample = MOCKED_SAMPLE})
#define DELAY         1

static void
dummy_callback_not_null(struct dummy_struct* args) {
    assert_not_null(args);
    assert_int(args->sample, ==, MOCKED_SAMPLE);
}

static void
dummy_no_call(void* args) {
    assert_true(false); // not meant to be called
}

TEARDOWN(tear_down_test) {}

TEST(test_nordi_new_immediate_success_null) {
    nordi_routine_ptr routine = nordi_routine_new(dummy_callback_null, NULL, 0);
    assert_not_null(routine);
    assert_int(routine->delay, ==, 0);
    assert_null(routine->context);
    assert_ptr_equal(routine->function, dummy_callback_null);
    nordi_routine_join(routine);
    assert_int(routine->state, ==, FINISH);
}

TEST(test_nordi_new_immediate_success_not_null) {
    struct dummy_struct mock = MOCKED_STRUCT;
    nordi_routine_ptr routine = nordi_routine_new(dummy_callback_not_null, &mock, 0);
    assert_not_null(routine);
    assert_int(routine->delay, ==, 0);
    assert_ptr_equal(routine->context, &mock);
    assert_ptr_equal(routine->function, dummy_callback_not_null);
    nordi_routine_join(routine);
    assert_int(routine->state, ==, FINISH);
}

TEST(test_nordi_new_immediate_success_cancel) {
    nordi_routine_ptr routine = nordi_routine_new(dummy_callback_null, NULL, 0);
    assert_not_null(routine);
    assert_int(routine->delay, ==, 0);
    assert_null(routine->context);
    assert_ptr_equal(routine->function, dummy_callback_null);
    nordi_routine_cancel(routine);
    assert_int(routine->state, ==, FINISH);
}

TEST(test_nordi_new_delayed_success_null) {
    nordi_routine_ptr routine = nordi_routine_new(dummy_callback_null, NULL, DELAY);
    time_t start = time(NULL);
    assert_not_null(routine);
    assert_int(routine->delay, ==, DELAY);
    assert_null(routine->context);
    assert_ptr_equal(routine->function, dummy_callback_null);
    nordi_routine_join(routine);
    int elapsed = (int)(time(NULL) - start);
    assert_int(elapsed, ==, DELAY);
    assert_int(routine->state, ==, FINISH);
}

TEST(test_nordi_new_delayed_success_not_null) {
    struct dummy_struct mock = MOCKED_STRUCT;
    nordi_routine_ptr routine = nordi_routine_new(dummy_callback_not_null, &mock, DELAY);
    time_t start = time(NULL);
    assert_not_null(routine);
    assert_int(routine->delay, ==, DELAY);
    assert_ptr_equal(routine->context, &mock);
    assert_ptr_equal(routine->function, dummy_callback_not_null);
    nordi_routine_join(routine);
    int elapsed = (int)(time(NULL) - start);
    assert_int(elapsed, ==, DELAY);
    assert_int(routine->state, ==, FINISH);
}

TEST(test_nordi_new_delayed_success_cancel) {
    nordi_routine_ptr routine = nordi_routine_new(dummy_no_call, NULL, DELAY);
    time_t start = time(NULL);
    assert_not_null(routine);
    assert_int(routine->delay, ==, DELAY);
    assert_null(routine->context);
    assert_ptr_equal(routine->function, dummy_no_call);
    nordi_routine_cancel(routine);
    int elapsed = (int)(time(NULL) - start);
    assert_int(routine->state, ==, CANCEL);
    assert_int(elapsed, <, DELAY);
}

TESTS(routine_tests) = {
    TESTRUN("/immediate-ok-no-args", test_nordi_new_immediate_success_null),
    TESTRUN("/immediate-ok-args", test_nordi_new_immediate_success_not_null),
    TESTRUN("/immediate-ok-cancel", test_nordi_new_immediate_success_cancel),
    TESTRUN("/delayed-ok-no-args", test_nordi_new_delayed_success_null),
    TESTRUN("/delayed-ok-args", test_nordi_new_delayed_success_not_null),
    TESTRUN("/delayed-ok-cancel", test_nordi_new_delayed_success_cancel),
    TESTEND,
};
