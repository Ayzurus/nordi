#ifndef MUNIT_ENABLE_ASSERT_ALIASES
#define MUNIT_ENABLE_ASSERT_ALIASES
#endif

#include "munit/munit.h"

#define SUITES(name) const MunitSuite name[]
#define SUITE(text, tests)                                                                                                                 \
    (const MunitSuite) { text, tests, NULL, 1, MUNIT_SUITE_OPTION_NONE }
#define TESTS(name)    const MunitTest name[]
#define TEST(name)     static MunitResult name(const MunitParameter params[], void* user_data)
#define SETUP(name)    static void* name(const MunitParameter params[], void* user_data)
#define TEARDOWN(name) static void name(void* fixture)
#define TESTRUN(log_text, test_name)                                                                                                       \
    { .name = log_text, .tear_down = tear_down_test, .test = test_name }
#define TESTEND                                                                                                                            \
    {}

extern TESTS(server_tests);
extern TESTS(api_tests);
extern TESTS(routine_tests);
