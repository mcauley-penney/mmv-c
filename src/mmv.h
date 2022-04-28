#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define TESTING

typedef u_int32_t Fnv32_t;

typedef struct str_pair
{
    char *dest;
    char *src;
    struct str_pair *next;
} pair;

Fnv32_t fnv_32a_str(char *str);
void free_map(pair *map[], const int keyarr[], const int keyarr_len);
void free_pair_ll(pair *node);
FILE *get_tmp_path_fptr(char *tmp_path);
int hashmap_insert(pair *map[], char *str, int hash);
pair *init_pair_node(char *src_str);
void open_file(char *path, char *mode, FILE **fptr);
void open_file_in_buf(const char *path, const int path_len);
void print_map(pair *map[], int keyarr[], int keyarr_len);
void read_lines_from_fptr(FILE *fptr, pair *map[], const int keyarr[], const int keyarr_len);
void rename_files(pair *map[], const int keyarr[], const int keyarr_len);
void rm_path(char *path);
void rm_str_nl(char *str);
void write_map_to_fptr(FILE *fptr, pair *map[], int keys[], const int num_keys);
