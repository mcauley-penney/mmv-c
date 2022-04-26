/**
 * TODO:
 * 1. produce documentation
 * 2. determine what datatype to use for numerical values here
 */

#include "mmv.h"

int main(int argc, char *argv[])
{
    char tmp_path[] = "/tmp/mmv_XXXXXX";
    FILE *tmp_fptr;
    int tmp_path_len = strlen(tmp_path);

    // remove program invocation from argv and decrement argc
    rm_strarr_index(argv, &argc, 0);

    if (argc > 1)
        rm_strarr_dupes(argv, argc);

    tmp_fptr = get_tmp_path_fptr(tmp_path);

    write_strarr_to_fptr(tmp_fptr, argv, argc);

    // there is no corresponding, explicit fopen() call for this fclose()
    // because mkstemp in mk_uniq_path() opens tmp file for us
    fclose(tmp_fptr);

    open_file_in_buf(tmp_path, tmp_path_len);

    char *new_name_arr[argc];

    open_file(tmp_path, "r", &tmp_fptr);

    // TODO: digest newlines in temp buffer
    read_strarr_from_fptr(tmp_fptr, argc, new_name_arr);

    fclose(tmp_fptr);

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

    rm_path(tmp_path);
    free_strarr(new_name_arr, argc);

    return EXIT_SUCCESS;
}

void rm_strarr_dupes(char *strarr[], int arr_len)
{
    int i, j;

    for (i = 0; i < arr_len; i++)
        for (j = i + 1; j < arr_len; j++)
            if (strcmp(strarr[j], strarr[i]) == 0)
                strcpy(strarr[j], "");
}

void rm_path(char *path)
{
    int rm_success = remove(path);

    if (rm_success == -1)
        fprintf(stderr, "ERROR: Unable to delete \"%s\"\n", path);
}

void free_strarr(char *strarr[], const int arg_count)
{
    int i;

    for (i = 0; i < arg_count; i++)
        free(strarr[i]);
}

void open_file(char *path, char *mode, FILE **fptr)
{
    *fptr = fopen(path, mode);

    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: Unable to open \"%s\" in \"%s\" mode\n", path, mode);
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

FILE *get_tmp_path_fptr(char *tmp_path)
{
    FILE *fptr;
    int tmp_fd;

    tmp_fd = mkstemp(tmp_path);

    if (tmp_fd == -1)
    {
        fprintf(stderr, "ERROR: unable to open \"%s\" as file descriptor\n", tmp_path);
        exit(EXIT_FAILURE);
    }

    fptr = fdopen(tmp_fd, "w");

    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: unable to open \"%s\" as file pointer\n", tmp_path);
        exit(EXIT_FAILURE);
    }

    return fptr;
}

void read_strarr_from_fptr(FILE *fptr, char *strarr[], const int arg_count)
{
    const int max_str_len = 500;
    char *cur_str = malloc(max_str_len * sizeof(cur_str)), *read_ptr = "";
    int i = 0, j = 0;

    while (i < arg_count && read_ptr != NULL)
    {
        read_ptr = fgets(cur_str, max_str_len, fptr);

        if (read_ptr != NULL && strcmp("\n", cur_str) != 0)
        {
            strarr[j] = malloc(max_str_len * sizeof(strarr[j]));
            rm_str_nl(cur_str);
            strcpy(strarr[j], cur_str);
            j++;
            i++;
        }
    }

    free(cur_str);
}

void rename_files(char *old_nm_arr[], char *new_nm_arr[], const int arg_count)
{
    int i, rename_result;

    for (i = 0; i < arg_count; i++)
    {
        rename_result = rename(old_nm_arr[i], new_nm_arr[i]);

        if (rename_result == -1)
            fprintf(stderr, "ERROR: Could not rename \"%s\" to \"%s\"", old_nm_arr[i], new_nm_arr[i]);
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

void write_strarr_to_fptr(FILE *fptr, char *args[], const int arg_count)
{
    int i;

    for (i = 0; i < arg_count; i++)
        if (strlen(args[i]) > 0)
            fprintf(fptr, "%s\n", args[i]);
}

void print_strarr(char *strarr[], const int arr_len)
{
    int i;

    for (i = 0; i < arr_len; i++)
        printf("index %d: %s\n", i, strarr[i]);

    printf("\n");
}
