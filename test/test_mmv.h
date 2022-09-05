#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void verify_correct_set_creation(void);
void assert_set_null(int argc, char *argv[]);
void assert_set_size_and_content(int argc, size_t correct_argc, char *argv[], char *correct_argv[]);
void verify_correct_write_to_file(void);
