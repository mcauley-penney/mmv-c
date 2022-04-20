#include "mmv.h"

int main(int argc, char *argv[])
{
    char *tmp_path = NULL;
    int tmp_path_len;

    // remove program invocation from argv and decrement argc
    rm_strarr_index(argv, &argc, 0);

    tmp_path = mk_uniq_path("/tmp/mmv_", ".txt", &tmp_path_len);

    // open file for writing
    write_strarr_to_file(tmp_path, argv, argc);

    // open in buffer
    open_file_in_buf(tmp_path, tmp_path_len);

    destroy_path(tmp_path);

    return EXIT_SUCCESS;
}

void destroy_path(char *path)
{
    int rm_success;

    rm_success = remove(path);

    if (rm_success == -1)
        printf("ERROR: Unable to delete \"%s\"", path);

    free(path);
}

void open_file(char *path, char *mode, FILE **fptr)
{
    *fptr = malloc(sizeof(FILE));
    *fptr = fopen(path, mode);

    if (fptr == NULL)
    {
        printf("ERROR: Unable to open \"%s\" in \"%s\" mode\n", path, mode);
        exit(EXIT_FAILURE);
    }
}

void open_file_in_buf(const char *path, const int path_len)
{
    char *editor_cmd = "$EDITOR";
    int edit_cmd_len = strlen(editor_cmd) + path_len + 1;
    char *edit_cmd = malloc(edit_cmd_len * sizeof(char));

    sprintf(edit_cmd, "%s %s", editor_cmd, path);

    // open temporary file containing argv using editor of choice
    system(edit_cmd);

    free(edit_cmd);
}

char *mk_uniq_path(const char *prefix, const char *ext, int *path_len)
{
    int mod = 100000, suffix, suffix_prec;
    char *path = NULL;

    srand(time(0));

    suffix = rand() % mod;

    // get "len" of randomized digit suffix for mem allocation
    suffix_prec = floor(log10(suffix)) + 1;

    *path_len = strlen(prefix) + strlen(ext) + suffix_prec + 1;

    path = malloc(*path_len * sizeof(char));
    sprintf(path, "%s%d%s", prefix, suffix, ext);

    return path;
}

void rm_strarr_index(char *strarr[], int *arr_len, int index)
{
    int i;

    for (i = index + 1; i < *arr_len; i++)
        strarr[i - 1] = strarr[i];

    (*arr_len)--;
}

void write_strarr_to_file(char *path, char *args[], const int arg_count)
{
    FILE *fptr = NULL;
    int i;

    open_file(path, "w", &fptr);

    for (i = 0; i < arg_count; i++)
        fprintf(fptr, "%s", args[arg_count]);

    fclose(fptr);
}
