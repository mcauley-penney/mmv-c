#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void destroy_path(char *path);
char *mk_uniq_path(const char *prefix, const char *ext, int *path_len);
void open_file(char *path, char *mode, FILE **fptr);
void open_file_in_buf(const char *path, const int path_len);
void rm_strarr_index(char *strarr[], int *arr_len, int index);
void write_strarr_to_file(char *path, char *args[], const int arg_count);
