#ifndef SET_H
#define SET_H


#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./utils.h"


#define MAX_OPS 50000

typedef u_int32_t Fnv32_t;

struct Set
{
    char **map;
    unsigned long int map_capacity;
    unsigned int num_keys;
    int keys[MAX_OPS + 1];
};


/**
 * @brief Create a set of strings from argv
 *
 * @param argv
 * @param argc
 * @param ***map: pointer to a string array
 * @param **key: pointer to MapKeyArr struct
 */
struct Set *set_init(bool resolve_paths, const int arg_count, char *args[], bool track_dupes);

/**
 * @brief completely frees a Map struct
 *
 * @param map: Map struct to free the nodes of
 */
void set_destroy(struct Set *set);

/* TODO: */
int is_duplicate_element(char *cur_str, struct Set *set, long unsigned int *hash);

int *set_begin(struct Set *set);

int *set_next(int *iter);

int *set_end(struct Set *set);

char **get_set_pos(const struct Set *set, const int *iter);

int is_valid_key(const int *iter);

int set_key(int *iter, int new_key);

#endif // SET_H
