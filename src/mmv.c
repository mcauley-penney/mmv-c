/**
 * TODO:
 * 1. produce documentation
 */

#include "mmv.h"

int main(int argc, char *argv[])
{
    char *tmp_path = NULL;
    int tmp_path_len;

    // remove program invocation from argv and decrement argc
    rm_strarr_index(argv, &argc, 0);

    char *new_name_arr[argc];

    // TODO: remove duplicate inputs
    //  - start with str comparison
    //  - advance to some higher form, like hashing

    tmp_path = mk_uniq_path("/tmp/mmv_", ".txt", &tmp_path_len);

    write_strarr_to_file(tmp_path, argv, argc);

    open_file_in_buf(tmp_path, tmp_path_len);

    // TODO: digest newlines in temp buffer
    read_strarr_from_file(tmp_path, tmp_path_len, new_name_arr);

    /**
     * TODO: add protections
     *  - disallow duplicate renames
     *  - allow swapping and cycling names
     *  - make sure we can handle some common escape chars in names
     *      - \n
     *
     *  - Idea: rename all items to temp names then proceed with rename to
     *          chosen new name
     *          - can this be done on a per-filename basis?
     */

    rename_files(argv, new_name_arr, argc);

    destroy_path(tmp_path);
    free_strarr(new_name_arr, argc);

    return EXIT_SUCCESS;
}

void destroy_path(char *path)
{
    int rm_success;

    rm_success = remove(path);

    if (rm_success == -1)
        printf("ERROR: Unable to delete \"%s\"\n", path);

    free(path);
}

void free_strarr(char *strarr[], const int arg_count)
{
    int i;

    for (i = 0; i < arg_count; i++)
        free(strarr[i]);
}

void open_file(char *path, char *mode, FILE **fptr)
{
    *fptr = malloc(sizeof(fptr));
    *fptr = fopen(path, mode);

    if (fptr == NULL)
    {
        printf("ERROR: Unable to open \"%s\" in \"%s\" mode\n", path, mode);
        exit(EXIT_FAILURE);
    }
}

void open_file_in_buf(const char *path, const int path_len)
{
    char *editor_cmd = "$EDITOR ";
    int edit_cmd_len = strlen(editor_cmd) + path_len + 1;
    char *edit_cmd = malloc(edit_cmd_len * sizeof(edit_cmd));

    strcpy(edit_cmd, editor_cmd);
    strcat(edit_cmd, path);

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

    path = malloc(*path_len * sizeof(path));
    sprintf(path, "%s%d%s", prefix, suffix, ext);

    return path;
}

void read_strarr_from_file(char *path, const int arg_count, char *strarr[])
{
    const int max_str_len = 500;
    char *cur_str = malloc(max_str_len * sizeof(cur_str)), *read_ptr;
    FILE *fptr = NULL;
    int i, j = 0;

    open_file(path, "r", &fptr);

    for (i = 0; i < arg_count && read_ptr != NULL; i++)
    {
        read_ptr = fgets(cur_str, max_str_len, fptr);

        if (read_ptr != NULL)
        {
            strarr[j] = malloc(max_str_len * sizeof(strarr[j]));
            rm_str_nl(cur_str);
            strcpy(strarr[j], cur_str);
            j++;
        }
    }

    fclose(fptr);
    free(cur_str);
}

// TODO:
void rename_files(char *old_nm_arr[], char *new_nm_arr[], const int arg_count)
{
    int i, rename_result;

    for (i = 0; i < arg_count; i++)
    {
        char *old_nm = old_nm_arr[i];
        char *new_nm = new_nm_arr[i];

        rename_result = rename(old_nm, new_nm);

        if (rename_result == -1)
            printf("ERROR: Could not rename \"%s\" to \"%s\"", old_nm, new_nm);
    }
}

// https://stackoverflow.com/a/42564670
void rm_str_nl(char *str)
{
    char *end = str + strlen(str) - 1;

    while (end > str && isspace(*end))
        end--;

    *(end + 1) = '\0';
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
        fprintf(fptr, "%s\n", args[i]);

    fclose(fptr);
}
