#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TESTING

typedef u_int32_t Fnv32_t;

typedef struct str_pair
{
    char *dest;
    char *src;
    struct str_pair *next;
} pair;

void rename_path_pair(char *src, char *dest);
int map_find_src_pos(pair *map[], int hash, char *str);

Fnv32_t fnv_32a_str(char *str, int map_size);
void free_map(pair *map[], const int keyarr[], const int keyarr_len);
void free_pair_ll(pair *node);
FILE *get_tmp_path_fptr(char *tmp_path);
int hashmap_insert(pair *map[], char *str, int hash);
pair *init_pair_node(char *src_str);
void open_file(char *path, char *mode, FILE **fptr);
void open_file_in_buf(const char *path, const int path_len);
void print_map(pair *map[], int keyarr[], int keyarr_len);
void read_lines_from_fptr(FILE *fptr, pair *map[], const int keyarr[], const int keyarr_len);
void rename_files(pair *map[], const int map_size, const int keyarr[], const int keyarr_len);
void rm_path(char *path);
void rm_str_nl(char *str);
void write_map_to_fptr(FILE *fptr, pair *map[], int keys[], const int num_keys);
