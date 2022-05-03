#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TESTING

typedef u_int32_t Fnv32_t;

struct StrPairNode
{
    char *src;
    char *dest;
    struct StrPairNode *next;
};

int attempt_strnode_map_insert(char *str, struct StrPairNode *map[], int map_size);
void free_map(struct StrPairNode *map[], const int keyarr[], const int keyarr_len);
void free_pair_ll(struct StrPairNode *node);
Fnv32_t get_fnv_32a_str_hash(char *str, int map_size);
int get_tmp_path_fd(char *tmp_path);
void handle_rename_collision(struct StrPairNode *map[], int map_size, char *cur_dest);
FILE *get_tmp_path_fptr(char *tmp_path);
struct StrPairNode *init_pair_node(char *src_str);
int map_find_src_pos(struct StrPairNode *map[], int hash, char *str);
void open_file(char *path, char *mode, FILE **fptr);
void open_tmp_file_in_editor(const char *path);
void print_map(struct StrPairNode *map[], int keyarr[], int keyarr_len);
void read_lines_from_fptr(FILE *fptr, struct StrPairNode *map[], const int keyarr[], const int keyarr_len);
void read_new_names_from_tmp_file(char tmp_path[], struct StrPairNode *map[], int keyarr[], int keyarr_len);
void rename_files(struct StrPairNode *map[], const int map_size, const int keyarr[], const int keyarr_len);
void rename_path_pair(char *src, char *dest);
void rm_path(char *path);
void rm_str_nl(char *str);
void write_map_to_fptr(FILE *fptr, struct StrPairNode *map[], int keys[], const int num_keys);
void write_old_names_to_tmp_file(char tmp_path[], struct StrPairNode *map[], int keyarr[], int keyarr_len);
