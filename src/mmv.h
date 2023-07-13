/**
 *  Title       : mmv-c
 *  Description : interactively move files and directories
 *  Author      : Jacob M. Penney
 */


#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PROG_NAME    "mmv"
#define PROG_VERSION "version 0.2.0"
#define PROG_REMOTE  "https://github.com/mcauley-penney/mmv-c"
#define MAX_OPS      50000

typedef u_int32_t Fnv32_t;

struct Opts
{
	bool resolve_paths;
	bool verbose;
};

struct Set
{
	char **map;
	unsigned long int map_capacity;
	unsigned int num_keys;
	int keys[MAX_OPS + 1];
};


/**
 * @brief Create struct of possible user flags
 * @return Opt struct
 */
struct Opts *make_opts(void);

/**
 * @brief Print program usage information
 */
void usage(void);

/**
 * @brief Print program help information
 */
void try_help(void);

/**
 * @brief Create a set of strings from argv
 *
 * @param argv
 * @param argc
 * @param ***map: pointer to a string array
 * @param **key: pointer to MapKeyArr struct
 */
struct Set *set_init(
    bool resolve_paths, const int arg_count, char *args[], bool track_dupes
);

/**
 * @brief Allocate memory for members of a Set struct
 *
 * @param num_args
 * @param map_size
 */
struct Set *set_alloc(const unsigned long int map_size);

/**
 * @brief Insert a string into the given Set struct
 *
 * @param cur_str
 * @param map_size
 * @param map
 * @return
 */
int set_insert(char *cur_str, struct Set *set, bool track_dupes);

/**
 * @brief hashes a string with the Fowler–Noll–Vo 1a 32bit hash fn
 *
 * @param str: string to hash
 * @param map_size: size of map; used to modulo the hash to fit into array
 *
 * @return hash for input string % map_size
 */
long unsigned int hash_str(char *str, const unsigned long int map_size);

/**
 * @brief move the given source string into the given hash map position
 *
 * @param array_pos: position in hash map to populate
 * @param src_str: str to copy into hash map
 */
char *cpy_str_to_arr(char **array_pos, const char *src_str);

/**
 * @brief opens temp file at path, writes source strings (old names)
 *      to it, and closes said temp file
 *
 * @param path: path to open and write to
 * @param map: map of source strings to write
 * @param keys: struct containing list of keys to node locations in
 *      hashmap
 */
int write_strarr_to_tmpfile(struct Set *map, char tmp_path_template[]);


/**
 * @brief opens the given path and returns its file pointer
 *
 * @param tmp_path: string, file path to open
 *
 * @return file pointer to opened path
 */
FILE *__attribute__((malloc)) open_tmpfile_fptr(char *tmp_path);

/**
 * @brief gets the user's $EDITOR env variable and opens temp file with it
 *
 * @param path: file path to open in editor
 */
int edit_tmpfile(const char *path);

struct Set *init_dest_set(unsigned int num_keys, char path[]);


/**
 * TOOD:
 * @brief reads lines out of a file path and renames item at corresponding
 *      position in hashmap to newly-read line.
 *
 *  For example, the first item in the hashmap will be renamed to the first
 *  item read from the file.
 *
 * @param options: struct of user-defined flags
 * @param set: set of strings to rename
 * @param path: path to temp file containing new names
 * @return errno or 0 for success
 */
int read_tmpfile_strs(
    char **dest_arr, int *dest_size, unsigned int num_keys, char path[]
);


void free_strarr(char **arr, int arr_size);

int rename_paths(
    struct Set *src_set, struct Set *dest_set, struct Opts *options
);

/**
 * @brief renames an item in file system
 *
 * @param options: struct of user-defined flags
 * @param src: old name
 * @param dest: new name
 */
void rename_path(const char *src, const char *dest, struct Opts *options);

/**
 * @brief deletes an item at the given path in the file system
 *
 * @param path: path to item to delete
 */
void rm_path(char *path);

/**
 * @brief completely frees a Map struct
 *
 * @param map: Map struct to free the nodes of
 */
void set_destroy(struct Set *map);

int rm_cycles(struct Set *src_set, struct Set *dest_set, struct Opts *options);

int is_duplicate_element(
    char *cur_str, struct Set *set, long unsigned int *hash
);
