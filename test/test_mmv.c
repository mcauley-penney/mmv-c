#include <unistd.h>

#include "../src/mmv.c"
#include "../src/set.c"
#include "../src/utils.c"
#include "unity.h"

#define DEBUG 1

void setUp(void)
{
}

void tearDown(void)
{
}

static struct Opts *make_opts(void)
{
    struct Opts *opts = malloc(sizeof(struct Opts));
    if (opts == NULL)
    {
        perror("mmv: failed to allocate memory for user flags\n");
        return NULL;
    }

    opts->resolve_paths = false;
    opts->verbose       = false;

    return opts;
}

void test_rm_path(void)
{
    char *path = "MMV_TEST_FILE";
    FILE *fptr = fopen(path, "w");
    TEST_ASSERT_NOT_NULL(fptr);

    fclose(fptr);
    rm_path(path);

    int exists = access(path, F_OK);
    TEST_ASSERT(exists != 0);
}

void test_open_tmpfile_fptr_incorrect_path(void)
{
    char path[] = "mmv_test";
    FILE *fptr  = open_tmpfile_fptr(path);

    TEST_ASSERT_NULL(fptr);
}

void test_open_tmpfile_fptr_correct_path(void)
{
    char path[] = "mmv_XXXXXX";
    FILE *fptr  = open_tmpfile_fptr(path);

    fclose(fptr);
    rm_path(path);

    TEST_ASSERT_NOT_NULL(fptr);
}

void test_rename_path(void)
{
    struct Opts *options = make_opts();

    char *src = "MMV_TEST_FILE1", *dest = "MMV_TEST_FILE2";
    FILE *fptr = fopen(src, "w");
    TEST_ASSERT_NOT_NULL(fptr);

    fclose(fptr);

    rename_path(src, dest, options);

    int exists = access(dest, F_OK);

    free(options);
    rm_path(dest);

    TEST_ASSERT_EQUAL_INT(0, exists);
}

void test_write_strarr_to_tmpfile(void)
{
    int argc                = 2;
    char *empty_argv[]      = {"TEST1", "TEST2"};
    char correct_tmp_path[] = "mmv_XXXXXX";

    struct Set *test_set = set_init(false, argc, empty_argv, false);

    int ret = write_strarr_to_tmpfile(test_set, correct_tmp_path);
    rm_path(correct_tmp_path);
    set_destroy(test_set);
    TEST_ASSERT(ret == 0);
}

void test_edit_tmpfile(void)
{
    char path[] = "mmv_XXXXXX";
    FILE *fptr  = open_tmpfile_fptr(path);
    fclose(fptr);

    int ret = edit_tmpfile(path);

    rm_path(path);
    TEST_ASSERT_EQUAL_INT(0, ret);
}

void test_read_tmpfile_strs(void)
{
    int argc        = 2;
    char *argv[]    = {"TEST1", "TEST2"};
    char tmp_path[] = "mmv_test_XXXXXX";
    char **dest_arr = malloc(sizeof(char *) * argc);
    if (dest_arr == NULL)
        TEST_FAIL();

    struct Set *test_set = set_init(false, argc, argv, false);
    write_strarr_to_tmpfile(test_set, tmp_path);

    int dest_size = 0;

    if (read_tmpfile_strs(dest_arr, &dest_size, argc, tmp_path) != 0)
    {
        free_strarr(dest_arr, dest_size);
        rm_path(tmp_path);
        set_destroy(test_set);
        TEST_FAIL();
    }

    rm_path(tmp_path);
    TEST_ASSERT_EQUAL_STRING_ARRAY(argv, dest_arr, argc);
    free_strarr(dest_arr, dest_size);
    set_destroy(test_set);
}

void test_rm_cycles()
{
    struct Opts *options = make_opts();

    int *i;
    char *src_str;

    int src_argc        = 2;
    char *src_argv[]    = {"TEST STRING1", "TEST STRING2"};
    struct Set *src_set = set_init(false, src_argc, src_argv, false);

    int dest_argc        = 2;
    char *dest_argv[]    = {"TEST STRING2", "TEST STRING3"};
    struct Set *dest_set = set_init(false, dest_argc, dest_argv, false);

    rm_cycles(src_set, dest_set, options);

    for (i = set_begin(src_set); i < set_end(src_set) - 1; i = set_next(i))
        ;
    src_str = *get_set_pos(src_set, i);
    TEST_ASSERT(strcmp(src_argv[1], src_str) != 0);

    rm_path(src_str);
    free(options);
    set_destroy(src_set);
    set_destroy(dest_set);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_rm_path);                          // rm_path
    RUN_TEST(test_open_tmpfile_fptr_incorrect_path); // open_tmpfile_fptr
    RUN_TEST(test_open_tmpfile_fptr_correct_path);   // open_tmpfile_fptr
    RUN_TEST(test_rename_path);                      // rename_path
    RUN_TEST(test_write_strarr_to_tmpfile);          // write_strarr_to_tmpfile
    RUN_TEST(test_edit_tmpfile);                     // edit_tmpfile
    RUN_TEST(test_read_tmpfile_strs);                // read_tmpfile_strs
    RUN_TEST(test_rm_cycles);                        // rm_cycles

    return UNITY_END();
}
