#include "../src/utils.c"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_cpy_str_to_arr(void)
{
    char *test_arr[] = {NULL};
    char *test_str   = "TEST STRING\0";
    cpy_str_to_arr(&test_arr[0], test_str);
    TEST_ASSERT_EQUAL_STRING("TEST STRING", test_arr[0]);

    free(test_arr[0]);
    test_str = "\0";
    cpy_str_to_arr(&test_arr[0], test_str);
    TEST_ASSERT_EQUAL_STRING("", test_arr[0]);
}

void test_strccat_no_args(void)
{
    char *parts[7] = {"This", " ", "is", " ", "a", " ", "test"};

    char *test_str = strccat(parts, 0);

    TEST_ASSERT_NULL(test_str);
}

void test_strccat_incomplete_args(void)
{
    char *parts[7] = {"This", " ", "is", " ", "a", " ", "test"};

    char *test_str = strccat(parts, 3);

    TEST_ASSERT_EQUAL_STRING("This is", test_str);
}

void test_strccat_excess_args(void)
{
    char *parts[7] = {"This", " ", "is", " ", "a", " ", "test"};

    char *test_str = strccat(parts, 8);

    TEST_ASSERT_EQUAL_STRING("This is a test", test_str);
}

void test_strccat_successful(void)
{
    char *parts[7] = {"This", " ", "is", " ", "a", " ", "test"};

    char *test_str = strccat(parts, 7);

    TEST_ASSERT_EQUAL_STRING("This is a test", test_str);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_cpy_str_to_arr);
    RUN_TEST(test_strccat_no_args);
    RUN_TEST(test_strccat_incomplete_args);
    RUN_TEST(test_strccat_successful);

    return UNITY_END();
}
