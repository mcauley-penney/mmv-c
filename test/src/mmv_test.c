#include "mmv_test.h"
#include "../../src/mmv.h"
#include "../../unity/unity.h"

// TODO: separate out into more tests, one per map
// make it a suite?
void assert_hashmap_correct_creation()
{
    int argc = 11;
    char *argv_11[11] = {"TEST_INVOCATION", "test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt",
                         "test6.txt",       "test7.txt", "test8.txt", "test9.txt", "test10.txt"};

    assert_hashmap_size_and_content(argc, argv_11);

    argc = 1;
    char *argv_1[1] = {"TEST_INVOCATION"};

    struct Map *test_map = make_str_hashmap(argc, argv_1);

    TEST_ASSERT_NULL(test_map);
}

void assert_hashmap_size_and_content(int argc, char *argv[])
{
    char *cur_str;
    int expected_argc = argc - 1, i;

    struct Map *test_map = make_str_hashmap(argc, argv);

    TEST_ASSERT_EQUAL_UINT(expected_argc, test_map->num_keys);

    // iterate over argv and each string at i must match
    for (i = 0; i < expected_argc; i++)
    {
        cur_str = test_map->hashmap[test_map->keyarr[i]];
        TEST_ASSERT_EQUAL_STRING(argv[i + 1], cur_str);
    }
}

void setUp(void)
{
    // TODO: include umask?

    // can contain anything you would like to run before each test
    // accepts no arguments, returns nothing
}

void tearDown(void)
{
    // can contain anything you would like to run after each test
    // accepts no arguments, returns nothing
}

int main(int argc, char *argv[])
{
    UNITY_BEGIN();
    RUN_TEST(assert_hashmap_correct_creation);
    return UNITY_END();
}

// void assert
