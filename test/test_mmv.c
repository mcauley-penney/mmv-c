#include "test_mmv.h"
#include "../src/mmv.h"

void assert_hashmap_correct_creation(void)
{
    int argc = 11;
    char *argv_11[11] = {"TEST_INVOCATION", "test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt",
                         "test6.txt",       "test7.txt", "test8.txt", "test9.txt", "test10.txt"};

    assert_hashmap_size_and_content(argc, argv_11);

    argc = 1;
    char *argv[1] = {"TEST_INVOCATION"};

    struct Set *test_set = make_str_set(argc, argv);

    assert(test_set == NULL);
}

void assert_hashmap_size_and_content(int argc, char *argv[])
{
    char *cur_str;
    size_t expected_argc = (size_t)argc - 1, i;

    struct Set *test_set = make_str_set(argc, argv);

    assert(expected_argc == test_set->num_keys);

    // iterate over argv and each string at i must match
    for (i = 0; i < expected_argc; i++)
    {
        cur_str = test_set->map[test_set->keys[i]];
        assert(strcmp(argv[i + 1], cur_str) == 0);
    }
}
