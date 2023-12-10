/**
 *  Title       : mmv-c
 *  Description : interactively move files and directories
 *  Author      : Jacob M. Penney
 */


#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./set.h"
#include "./utils.h"


#define PROG_NAME    "mmv"
#define PROG_VERSION "version 0.3.2"
#define PROG_REMOTE  "https://github.com/mcauley-penney/mmv-c"

struct Opts
{
    bool resolve_paths;
    bool verbose;
};

struct Set *init_src_set(const int arg_count, char *args[], struct Opts *options);

/**
 * @brief opens temp file at path, writes source strings (old names)
 *      to it, and closes said temp file
 *
 * @param path: path to open and write to
 * @param map: map of source strings to write
 * @param keys: struct containing list of keys to node locations in
 *      hashmap
 */
int write_strarr_to_tmpfile(struct Set *set, char tmp_path_template[]);

/**
 * @brief gets the user's $EDITOR env variable and opens temp file with it
 *
 * @param path: file path to open in editor
 */
int edit_tmpfile(char *path);

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
int read_tmpfile_strs(char **dest_arr, int *dest_size, unsigned int num_keys, char path[]);


void free_strarr(char **arr, int arr_size);

int rename_paths(struct Set *src_set, struct Set *dest_set, struct Opts *options);

/**
 * @brief renames an item in file system
 *
 * @param options: struct of user-defined flags
 * @param src: old name
 * @param dest: new name
 */
void rename_path(const char *src, const char *dest, struct Opts *options);

int rm_unedited_pairs(struct Set *src_set, struct Set *dest_set, struct Opts *opts);

int rm_cycles(struct Set *src_set, struct Set *dest_set, struct Opts *options);
