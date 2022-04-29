#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TESTING

typedef u_int32_t Fnv32_t;

// struct KeysData
// {
//     int key_arrlen;
//     int key_arr[];
// };
//
// struct Map
// {
//     struct KeysData *key_obj;
//     struct StrPairNode *map_arr[];
// };

struct StrPairNode
{
    char *src;
    char *dest;
    struct StrPairNode *next;
};

void init_name_hashmap(char *args_arr[], int args_arrlen, struct StrPairNode *map[], int map_len, int key_arr[],
                       int *key_arrlen);
void read_new_names_from_tmp_file(char tmp_path[], struct StrPairNode *map[], int keyarr[], int keyarr_len);
void write_old_names_to_tmp_file(char tmp_path[], struct StrPairNode *map[], int keyarr[], int keyarr_len);

void rename_path_pair(char *src, char *dest);
int map_find_src_pos(struct StrPairNode *map[], int hash, char *str);

Fnv32_t get_fnv_32a_str_hash(char *str, int map_size);
void free_map(struct StrPairNode *map[], const int keyarr[], const int keyarr_len);
void free_pair_ll(struct StrPairNode *node);
FILE *get_tmp_path_fptr(char *tmp_path);
int hashmap_insert(struct StrPairNode *map[], char *str, int hash);
struct StrPairNode *init_pair_node(char *src_str);
void open_file(char *path, char *mode, FILE **fptr);
void open_tmp_file_in_editor(const char *path);
void print_map(struct StrPairNode *map[], int keyarr[], int keyarr_len);
void read_lines_from_fptr(FILE *fptr, struct StrPairNode *map[], const int keyarr[], const int keyarr_len);
void rename_files(struct StrPairNode *map[], const int map_size, const int keyarr[], const int keyarr_len);
void rm_path(char *path);
void rm_str_nl(char *str);
void write_map_to_fptr(FILE *fptr, struct StrPairNode *map[], int keys[], const int num_keys);
