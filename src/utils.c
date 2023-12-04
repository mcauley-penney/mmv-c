#include "./utils.h"

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
