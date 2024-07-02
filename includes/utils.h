#ifndef UTILS_H
#define UTILS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief move the given source string into the given hash map position
 *
 * @param array_pos: position in hash map to populate
 * @param src_str: str to copy into hash map
 */
char *cpy_str_to_arr(char **array_pos, const char *src_str);

char *strccat(char **str_arr, unsigned int num_strs);

#endif // UTILS_H
