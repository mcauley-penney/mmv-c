#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_strarr(char *strarr[], const int arg_count);
FILE *get_tmp_path_fptr(char *tmp_path);
void open_file(char *path, char *mode, FILE **fptr);
void open_file_in_buf(const char *path, const int path_len);
void read_strarr_from_fptr(FILE *fptr, const int arg_count, char *strarr[]);
void rename_files(char *old_nm_arr[], char *new_nm_arr[], const int arg_count);
void rm_path(char *path);
void rm_str_nl(char *str);
void rm_strarr_dupes(char *strarr[], int arr_len);
void rm_strarr_index(char *strarr[], int *arr_len, int index);
void write_strarr_to_fptr(FILE *fptr, char *args[], const int arg_count);
