#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void destroy_path(char *path);
void free_strarr(char *strarr[], const int arg_count);
char *mk_uniq_path(const char *prefix, const char *ext, int *path_len);
void open_file(char *path, char *mode, FILE **fptr);
void open_file_in_buf(const char *path, const int path_len);
void read_strarr_from_file(char *path, const int arg_count, char *strarr[]);
void rename_files(char *old_nm_arr[], char *new_nm_arr[], const int arg_count);
void rm_str_nl(char *str);
void rm_strarr_index(char *strarr[], int *arr_len, int index);
void write_strarr_to_file(char *path, char *args[], const int arg_count);
