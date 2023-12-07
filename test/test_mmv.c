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
    remove(dest);

    TEST_ASSERT_EQUAL_INT(0, exists);
}

void test_write_strarr_to_tmpfile(void)
{
    int argc                = 2;
    char *empty_argv[]      = {"TEST1", "TEST2"};
    char correct_tmp_path[] = "mmv_XXXXXX";

    struct Set *test_set = set_init(false, argc, empty_argv, false);

    int ret = write_strarr_to_tmpfile(test_set, correct_tmp_path);
    remove(correct_tmp_path);
    set_destroy(test_set);
    TEST_ASSERT(ret == 0);
}

void test_edit_tmpfile(void)
{
    char path[] = "mmv_XXXXXX";
    int tmp_fd  = mkstemp(path);
    if (tmp_fd == -1)
        fprintf(stderr, "mmv: could not create temporary file \'%s\': %s\n", path, strerror(errno));

    FILE *fptr = fdopen(tmp_fd, "w");
    fclose(fptr);

    int ret = edit_tmpfile(path);

    remove(path);
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
        remove(tmp_path);
        set_destroy(test_set);
        TEST_FAIL();
    }

    remove(tmp_path);
    TEST_ASSERT_EQUAL_STRING_ARRAY(argv, dest_arr, argc);
    free_strarr(dest_arr, dest_size);
    set_destroy(test_set);
}

void test_rm_unedited_pairs_no_matches(void)
{
    int *i;

    struct Opts *options = make_opts();

    // 1. create two sets
    int src_argc        = 2;
    char *src_argv[]    = {"TEST_STRING1", "TEST_STRING2"};
    struct Set *src_set = set_init(false, src_argc, src_argv, false);

    int dest_argc        = 2;
    char *dest_argv[]    = {"TEST_STRING2", "TEST_STRING3"};
    struct Set *dest_set = set_init(false, dest_argc, dest_argv, false);

    // 2. subject them to rm_unedited_pairs()
    rm_unedited_pairs(src_set, dest_set, options);

    // 3. check their keys
    for (i = set_begin(dest_set); i < set_end(dest_set) - 1; i = set_next(i))
        TEST_ASSERT(*i != -1);
}

void test_rm_unedited_pairs_matches(void)
{
    int *i;

    struct Opts *options = make_opts();

    // 1. create two sets
    int src_argc        = 2;
    char *src_argv[]    = {"TEST_STRING1", "TEST_STRING2"};
    struct Set *src_set = set_init(false, src_argc, src_argv, false);

    int dest_argc        = 2;
    char *dest_argv[]    = {"TEST_STRING1", "TEST_STRING3"};
    struct Set *dest_set = set_init(false, dest_argc, dest_argv, false);

    // 2. subject them to rm_unedited_pairs()
    rm_unedited_pairs(src_set, dest_set, options);

    // 3. check their keys
    i = set_begin(dest_set);
    TEST_ASSERT_EQUAL_INT(-1, *i);
}

void test_rm_cycles(void)
{
    struct Opts *options = make_opts();

    int *i;
    char *src_str;

    int src_argc        = 2;
    char *src_argv[]    = {"TEST_STRING1", "TEST_STRING2"};
    struct Set *src_set = set_init(false, src_argc, src_argv, false);

    int dest_argc        = 2;
    char *dest_argv[]    = {"TEST_STRING2", "TEST_STRING3"};
    struct Set *dest_set = set_init(false, dest_argc, dest_argv, false);

    rm_cycles(src_set, dest_set, options);

    for (i = set_begin(src_set); i < set_end(src_set) - 1; i = set_next(i))
        ;
    src_str = *get_set_pos(src_set, i);
    TEST_ASSERT(strcmp(src_argv[1], src_str) != 0);

    free(options);
    set_destroy(src_set);
    set_destroy(dest_set);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_rename_path);                  // rename_path
    RUN_TEST(test_write_strarr_to_tmpfile);      // write_strarr_to_tmpfile
    RUN_TEST(test_edit_tmpfile);                 // edit_tmpfile
    RUN_TEST(test_read_tmpfile_strs);            // read_tmpfile_strs
    RUN_TEST(test_rm_unedited_pairs_no_matches); // rm_unedited_pairs
    RUN_TEST(test_rm_unedited_pairs_matches);    // rm_unedited_pairs
    RUN_TEST(test_rm_cycles);                    // rm_cycles

    return UNITY_END();
}
