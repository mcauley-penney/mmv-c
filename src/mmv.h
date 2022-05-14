#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef u_int32_t Fnv32_t;

struct StrPairNode
{
    char *src;
    char *dest;
    struct StrPairNode *next;
};

struct MapKeyArr
{
    int num_keys;
    int keyarr[];
};

/**
 * @brief recursively adds or rejects a node from the linked list of string
 * pair nodes found at a hashmap index
 *
 * @param cur_node: working node in iteration across linked list
 * @param new_src: source string (old name) to give to new node; will
 *      be rejected if a node is found to already contain it
 *
 * @return NULL or new node; if new node, new node is appended to end
 *      of linked list
 */
struct StrPairNode *add_strpair_node(struct StrPairNode *cur_node, const char *new_src);

/**
 * @brief attempts to add a source string to the hashmap of string pair
 *      nodes in a new node
 *
 * @param str: string to hash and add to map
 * @param map: map of string pair nodes to add string to
 * @param map_size: current size of the map
 *
 * @return hash where new node was inserted; -1 if key already exists
 */
int attempt_strnode_map_insert(char *str, struct StrPairNode *map[], int map_size);

/**
 * @brief hashes a string with the Fowler–Noll–Vo 1a 32bit hash fn
 *
 * @param str: string to hash
 * @param map_size: size of map; used to modulo the hash to fit into array
 *
 * @return hash after modulo
 */
int get_fnv_32a_str_hash(char *str, int map_size);

/**
 * @brief frees a hashmap of string pair nodes
 *
 * @param map: map to free
 * @param keys: struct containing list of keys where nodes exist
 */
void free_map(struct StrPairNode *map[], struct MapKeyArr *keys);

/**
 * @brief recursively frees all nodes in a linked list
 *
 * @param node: working node in recursion across linked list
 */
void free_pair_ll(struct StrPairNode *node);

/**
 * @brief opens the given path and gets its file pointer
 *
 * @param tmp_path: string of file path to open
 *
 * @return file pointer to opened path
 */
FILE *__attribute__((malloc)) get_tmp_path_fptr(char *tmp_path);

/**
 * @brief initializes and returns a string pair node
 *
 * @param src_str: string to give to node as source (old name) and
 *      temporarily as destination (new name)
 *
 * @return initialized node
 */
struct StrPairNode *init_pair_node(const char *src_str);

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

/**
 * @brief reads lines out of a file path and stores them in hashmap
 *      of string pair nodes as destination (new name) strings
 *
 * @param tmp_path: path to open and read lines from
 * @param map: map to store retrieved strings in
 * @param keys: struct containing list of keys of locations of
 *      string pair nodes in hashmap
 */
void read_new_names_from_tmp_file(char tmp_path[], struct StrPairNode *map[], struct MapKeyArr *keys);

/**
 * @brief iterates over hashmap of name pairs using list of keys and
 *      changes sources (old names) to destinations (new names)
 *
 * @param map: hashmap of old and new names
 * @param keys: struct containing list of keys to nodes in hashmap
 */
void rename_files(struct StrPairNode *map[], struct MapKeyArr *keys);

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
void write_map_to_fptr(FILE *fptr, struct StrPairNode *map[], struct MapKeyArr *keys);

/**
 * @brief opens temp file at path, writes source strings (old names)
 *      to it, and closes said temp file
 *
 * @param path: path to open and write to
 * @param map: map of source strings to write
 * @param keys: struct containing list of keys to node locations in
 *      hashmap
 */
void write_old_names_to_tmp_file(char path[], struct StrPairNode *map[], struct MapKeyArr *keys);
