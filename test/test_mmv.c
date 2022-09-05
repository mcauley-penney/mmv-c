#include "test_mmv.h"
#include "../src/mmv.h"
#include <math.h>

void verify_correct_set_creation(void)
{
    /**
     * Cases to test:
     *  1. number of inputs < 2
     *      • returns NULL
     *  2. No duplicate inputs
     *      • Resulting set has
     *          • (argc - 1) args
     *          • argv, without invocation, in set, in order
     *  3. Some duplicate inputs
     *      • Resulting set has
     *          • (argc - num_duplicates) args
     *          • argv, without invocation and duplicates, in set, in order
     **/

    int argc;
    size_t correct_argc;

    argc = 1;
    char *null_argv[1] = {"INVOCATION"};

    assert_set_null(argc, null_argv);

    argc = 11;
    correct_argc = 10;
    char *argv11[11] = {"INVOCATION", "a.out", "b.out", "c.out", "d.out", "e.out",
                        "f.out",      "g.out", "h.out", "i.out", "j.out"};
    char *correct_argv11[10] = {"a.out", "b.out", "c.out", "d.out", "e.out",
                                "f.out", "g.out", "h.out", "i.out", "j.out"};

    assert_set_size_and_content(argc, correct_argc, argv11, correct_argv11);

    argc = 11;
    correct_argc = 3;
    char *argv_duplicates[11] = {"INVOCATION", "test1.txt", "test1.txt", "test2.txt", "test2.txt", "test3.txt",
                                 "test1.txt",  "test2.txt", "test3.txt", "test1.txt", "test1.txt"};

    char *correct_argv_duplicates[3] = {"test1.txt", "test2.txt", "test3.txt"};

    assert_set_size_and_content(argc, correct_argc, argv_duplicates, correct_argv_duplicates);
}

void assert_set_null(int argc, char *argv[])
{
    struct Set *test_set = make_str_set(argc, argv);

    assert(test_set == NULL);
}

void assert_set_size_and_content(int argc, size_t correct_argc, char *argv[], char *correct_argv[])
{
    char *cur_str;
    size_t cur_key, i;

    struct Set *test_set = make_str_set(argc, argv);

    assert(correct_argc == test_set->num_keys);

    // iterate over argv and each string at i must match
    for (i = 0; i < correct_argc; i++)
    {
        cur_key = test_set->keys[i];
        cur_str = test_set->map[cur_key];
        assert(strcmp(correct_argv[i], cur_str) == 0);
    }

    free_str_set(test_set);
}

void verify_correct_write_to_file(void)
{
    int argc = 11;
    char *argv[11] = {"INVOCATION", "a.out", "b.out", "c.out", "d.out", "e.out",
                      "f.out",      "g.out", "h.out", "i.out", "j.out"};
    char tmp_path[] = "/tmp/mmv_XXXXXX";
    char cur_str[PATH_MAX], *read_ptr = "";
    size_t i = 0;

    struct Set *test_set = make_str_set(argc, argv);

    umask(077);

    assert(write_strarr_to_tmpfile(test_set, tmp_path) == 0);

    // Lifted out of reading function and modified
    FILE *tmp_fptr = fopen(tmp_path, "r");
    assert(tmp_fptr != NULL);

    while (read_ptr != NULL && i < test_set->num_keys)
    {
        read_ptr = fgets(cur_str, PATH_MAX, tmp_fptr);

        if (read_ptr != NULL && strcmp(cur_str, "\n") != 0)
        {
            cur_str[strlen(cur_str) - 1] = '\0';
            assert(strcmp(test_set->map[test_set->keys[i]], cur_str) == 0);
            i++;
        }
    }

    fclose(tmp_fptr);

    rm_path(tmp_path);
    free_str_set(test_set);
}
