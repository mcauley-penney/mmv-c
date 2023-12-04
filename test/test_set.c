#include "../src/set.c"
#include "../src/utils.c"
#include "unity.h"
#include <stdbool.h>

void setUp(void)
{
}

void tearDown(void)
{
}

/* tied to implementation details */
void test_set_alloc(void)
{
    struct Set *test_set = set_alloc(10);
    TEST_ASSERT_EQUAL_UINT(10, test_set->map_capacity);
    set_destroy(test_set);

    test_set = set_alloc(0);
    TEST_ASSERT_EQUAL_UINT(0, test_set->map_capacity);
    set_destroy(test_set);
}

void test_set_begin(void)
{
    int *i;
    char *cur_str, *test_str = "TEST STRING\0";

    struct Set *test_set = set_alloc(10);

    set_insert(test_str, test_set, true);
    i = set_begin(test_set);

    cur_str = *get_set_pos(test_set, i);
    TEST_ASSERT_EQUAL_STRING(test_str, cur_str);
}

void test_set_next(void)
{
    int *i;
    char *cur_str, *test_str1 = "TEST STRING1\0", *test_str2 = "TEST STRING2\0";

    struct Set *test_set = set_alloc(10);

    set_insert(test_str1, test_set, true);
    set_insert(test_str2, test_set, true);
    i = set_begin(test_set);
    i = set_next(i);

    cur_str = *get_set_pos(test_set, i);
    TEST_ASSERT_EQUAL_STRING(test_str2, cur_str);
}

void test_set_end(void)
{
    int *i;
    char *cur_str, *test_str1 = "TEST STRING1\0", *test_str2 = "TEST STRING2\0";
    struct Set *test_set = set_alloc(10);

    set_insert(test_str1, test_set, true);
    set_insert(test_str2, test_set, true);

    for (i = set_begin(test_set); i < set_end(test_set) - 1; i = set_next(i))
        ;

    cur_str = *get_set_pos(test_set, i);
    TEST_ASSERT_EQUAL_STRING(test_str2, cur_str);
}

void test_set_insert_uniq_str(void)
{
    char *cur_str, *test_str = "TEST STRING\0";

    struct Set *test_set = set_alloc(1);

    int insert_outcome = set_insert(test_str, test_set, true);
    TEST_ASSERT_EQUAL_INT(1, insert_outcome);
    set_destroy(test_set);
}

void test_set_insert_dup_str(void)
{
    char *cur_str, *test_str = "TEST STRING\0";

    struct Set *test_set = set_alloc(2);

    int insert_outcome = set_insert(test_str, test_set, true);
    TEST_ASSERT_EQUAL_INT(1, insert_outcome);

    insert_outcome = set_insert(test_str, test_set, true);
    TEST_ASSERT_EQUAL_INT(0, insert_outcome);

    set_destroy(test_set);
}

void test_set_init_empty(void)
{
    // case: NULL because no arguments
    int argc           = 0;
    char *empty_argv[] = {NULL};

    struct Set *test_set = set_init(false, argc, empty_argv, false);
    TEST_ASSERT_NULL(test_set);
}

void test_set_init_max(void)
{
    // case: NULL because too many arguments
    int argc           = MAX_OPS + 1;
    char *empty_argv[] = {NULL};

    struct Set *test_set = set_init(false, argc, empty_argv, false);
    TEST_ASSERT_NULL(test_set);
}

void test_set_init_onedupe_notrack(void)
{
    // case: two keys, one removed duplicate, no sentinal value
    int *i, argc                = 2;
    char *cur_str, *dupe_argv[] = {"TEST STRING", "TEST STRING"};

    struct Set *test_set = set_init(false, argc, dupe_argv, false);

    // make sure first string is present
    i       = set_begin(test_set);
    cur_str = *get_set_pos(test_set, i);
    TEST_ASSERT_EQUAL_STRING("TEST STRING", cur_str);

    // make sure second string has been removed
    i       = set_next(i);
    cur_str = *get_set_pos(test_set, i);
    TEST_ASSERT_NULL(cur_str);
    set_destroy(test_set);
}

void test_set_init_onedupe_track(void)
{
    // case: two keys, one removed duplicate, sentinal value
    int *i, argc                = 2;
    char *cur_str, *dupe_argv[] = {"TEST STRING", "TEST STRING"};

    struct Set *test_set = set_init(false, argc, dupe_argv, false);

    // make sure second string's key is sentinal value using predicate
    test_set = set_init(false, argc, dupe_argv, true);
    i        = set_begin(test_set);
    i        = set_next(i);
    TEST_ASSERT_TRUE(is_invalid_key(i));
    set_destroy(test_set);
}

int main(void)
{
    UNITY_BEGIN();

    // Set private helper functions
    RUN_TEST(test_set_alloc);
    RUN_TEST(test_set_insert_uniq_str);
    RUN_TEST(test_set_insert_dup_str);

    // Set iteration
    RUN_TEST(test_set_begin);
    RUN_TEST(test_set_next);
    RUN_TEST(test_set_end);

    // Set initialization
    RUN_TEST(test_set_init_empty);
    RUN_TEST(test_set_init_max);
    RUN_TEST(test_set_init_onedupe_notrack);
    RUN_TEST(test_set_init_onedupe_track);

    return UNITY_END();
}
