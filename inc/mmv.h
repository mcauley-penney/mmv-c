/**
 *  Title       : mmv-c
 *  Description : interactively move files and directories
 *  Author      : Jacob M. Penney
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef u_int32_t Fnv32_t;

struct Set
{
    size_t num_keys;
    char **map;
    unsigned int keys[];
};

/**
 * @brief Create a hashmap of strings from argv
 *
 * @param argv
 * @param argc
 * @param ***map: pointer to a string array
 * @param **key: pointer to MapKeyArr struct
 */
struct Set *make_str_set(int argc, char *argv[]);

struct Set *alloc_str_set(const unsigned int num_args, const unsigned int map_size);

int str_set_insert(char *cur_str, const unsigned int map_size, struct Set *map);

/**
 * @brief hashes a string with the Fowler–Noll–Vo 1a 32bit hash fn
 *
 * @param str: string to hash
 * @param map_size: size of map; used to modulo the hash to fit into array
 *
 * @return hash for input string % map_size
 */
unsigned int hash_str(char *str, const unsigned int map_size);

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
int write_strarr_to_tmpfile(struct Set *map, char tmppath[]);

/**
 * @brief opens the given path and returns its file pointer
 *
 * @param tmp_path: string, file path to open
 *
 * @return file pointer to opened path
 */
FILE *__attribute__((malloc)) open_tmp_path_fptr(char *tmp_path);

/**
 * @brief gets the user's $EDITOR env variable and opens temp file with it
 *
 * @param path: file path to open in editor
 */
int open_file_in_editor(const char *path);

/**
 * @brief reads lines out of a file path and renames item at corresponding
 *      position in hashmap to newly-read line.
 *
 *  For example, the first item in the hashmap will be renamed to the first
 *  item read from the file.
 *
 * @param tmp_path: path to open and read lines from
 * @param map: map to store retrieved strings in
 * @param keys: struct containing list of keys of locations of
 *      string pair nodes in hashmap
 */
int rename_filesystem_items(struct Set *map, char path[]);

/**
 * @brief renames an item in file system
 *
 * @param src: old name
 * @param dest: new name
 */
void rename_path(const char *src, const char *dest);

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
void free_str_set(struct Set *map);
