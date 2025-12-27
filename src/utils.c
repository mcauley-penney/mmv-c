#include "utils.h"

char *cpy_str_to_arr(char **arr_dest, const char *src_str)
{
    *arr_dest = malloc((strlen(src_str) + 1) * sizeof(char));
    if (arr_dest == NULL)
    {
        perror("mmv: failed to allocate memory for new map str\n");
        return NULL;
    }

    return strcpy(*arr_dest, src_str);
}

char *strccat(char **str_arr, unsigned int num_strs)
{
    if (num_strs < 1)
        return NULL;

    unsigned int i;
    size_t size      = 4200 * sizeof(char);
    char *concat_str = malloc(size);

    char *p = memccpy(concat_str, str_arr[0], '\0', size - 1);

    for (i = 1; i < num_strs; i++)
        if (p)
            p = memccpy(p - 1, str_arr[i], '\0', size - (size_t)p);
        else
        {
            free(concat_str);
            return NULL;
        }

    return concat_str;
}
