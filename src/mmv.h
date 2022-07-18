#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef u_int32_t Fnv32_t;

struct MapKeyArr
{
    size_t num_keys;
    unsigned int keyarr[];
};

/**
 * @brief hashes a string with the Fowler–Noll–Vo 1a 32bit hash fn
 *
 * @param str: string to hash
 * @param map_size: size of map; used to modulo the hash to fit into array
 *
 * @return hash for input string % map_size
 */
unsigned int calc_fnv32a_str_hash(char *str, const unsigned int map_size);

/**
 * @brief frees a hashmap of strings
 *
 * @param map: hash map to free the nodes of
 * @param keys: struct containing list of indices where nodes exist
 */
void free_map_nodes(char *map[], struct MapKeyArr *keys);

/**
 * @brief opens the given path and returns its file pointer
 *
 * @param tmp_path: string, file path to open
 *
 * @return file pointer to opened path
 */
FILE *__attribute__((malloc)) open_tmp_path_fptr(char *tmp_path);

void init_hashmap_objs(const unsigned int num_args, const unsigned int map_size, char ***map, struct MapKeyArr **keys);

/**
 * @brief move the given source string into the given hash map position
 *
 * @param array_pos: position in hash map to populate
 * @param src_str: str to copy into hash map
 */
void cp_str_to_arr(char **array_pos, const char *src_str);

/**
 * @brief Create a hashmap of strings from argv
 *
 * @param argv
 * @param argc
 * @param ***map: pointer to a string array
 * @param **key: pointer to MapKeyArr struct
 */
void make_argv_hashmap(char *argv[], int argc, char ***map, struct MapKeyArr **keys);

/**
 * @brief opens a file, given a mode and path to the file
 *
 * @param path: file path to open
 * @param mode: mode to open file in
 * @param fptr: out param, holds opened file
 */
void open_file(char *path, const char *mode, FILE **fptr);

/**
 * @brief gets the user's $EDITOR env variable and opens temp file with it
 *
 * @param path: file path to open in editor
 */
void open_tmp_file_in_editor(const char *path);

int probe_for_null_hashpos(char ***map, const unsigned int map_size, const char *cur_str, unsigned int *hash);

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
void rename_filesystem_items(char tmp_path[], char *map[], struct MapKeyArr *keys);

/**
 * @brief renames an item in file system
 *
 * @param src: old name
 * @param dest: new name
 */
void rename_path_pair(const char *src, const char *dest);

/**
 * @brief deletes an item at the given path in the file system
 *
 * @param path: path to item to delete
 */
void rm_path(char *path);

/**
 * @brief iterates over map of nodes containing item names to rename
 *      and writes said names to the given file pointer
 *
 * @param fptr: pointer to open file to write to
 * @param map: map of string pair nodes, contains old names to write
 *      to fptr
 * @param keys: struct containing list of keys to node locations in
 *      hashmap
 */
void write_map_to_fptr(FILE *fptr, char *map[], struct MapKeyArr *keys);

/**
 * @brief opens temp file at path, writes source strings (old names)
 *      to it, and closes said temp file
 *
 * @param path: path to open and write to
 * @param map: map of source strings to write
 * @param keys: struct containing list of keys to node locations in
 *      hashmap
 */
void write_old_names_to_tmp_file(char path[], char *map[], struct MapKeyArr *keys);
