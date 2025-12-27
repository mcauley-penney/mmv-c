#include "mmv.h"

int write_strarr_to_tmpfile(struct Set *set, char tmpfile_template[])
{
    int *i, *set_end_pos = set_end(set);

    int tmp_fd = mkstemp(tmpfile_template);
    if (tmp_fd == -1)
    {
        fprintf(stderr, "mmv: could not create temporary file \'%s\': %s\n", tmpfile_template, strerror(errno));
        return -1;
    }

    FILE *tmp_fptr = fdopen(tmp_fd, "w");

    for (i = set_begin(set); i < set_end_pos; i = set_next(i))
        if (is_valid_key(i))
            fprintf(tmp_fptr, "%s\n", *get_set_pos(set, i));

    fclose(tmp_fptr);

    return 0;
}

int edit_tmpfile(char *path)
{
    char *editor_name = getenv("EDITOR");
    if (editor_name == NULL)
        editor_name = "nano";

    char *cmd_parts[3] = {editor_name, " ", path};
    char *edit_cmd     = strccat(cmd_parts, 3);
    if (edit_cmd == NULL)
    {
        perror("mmv: failed to allocate memory for $EDITOR command string");
        return errno;
    }

#if DEBUG == 0
    if (system(edit_cmd) != 0)
    {
        fprintf(stderr, "mmv: \'%s\' returned non-zero exit status\n", editor_name);
        free(edit_cmd);
        return errno;
    }
#endif

    free(edit_cmd);

    return 0;
}

struct Set *init_src_set(const int num_keys, char *argv[], struct Opts *options)
{
    // prepare for duplicate removal by creating array of absolute paths from commandline args
    char **realpath_argv = malloc(sizeof(char *) * (unsigned int)num_keys);
    if (realpath_argv == NULL)
    {
        perror("mmv: failed to allocate memory for absolute path array");
        return NULL;
    }

    for (int i = 0; i < num_keys; i++)
        if (cpy_str_to_arr(&realpath_argv[i], realpath(argv[i], NULL)) == NULL)
        {
            free(realpath_argv);
            return NULL;
        }

    // turn array of absolute paths into a set to rm duplicates
    struct Set *realpath_set = set_init(options->resolve_paths, num_keys, realpath_argv, true);

    if (realpath_set == NULL)
    {
        free(realpath_argv);
        return NULL;
    }

    struct Set *src_set = realpath_set;

    // if not using the resolve paths opt, give the original arg
    // strings, those used on the commandline, back to the user.
    if (!options->resolve_paths)
    {
        int *key, *set_end_pos = set_end(realpath_set), key_num = 0;
        for (key = set_begin(realpath_set); key < set_end_pos; key = set_next(key))
            if (is_valid_key(key))
            {
                if (cpy_str_to_arr(&realpath_argv[key_num], argv[key_num]) == NULL)
                {
                    free(src_set);
                    return NULL;
                }

                key_num++;
            }

        src_set = set_init(false, key_num, realpath_argv, false);
    }

    free(realpath_argv);

    return src_set;
}

struct Set *init_dest_set(unsigned int num_keys, char path[])
{
    // size of destination array only needs to be, at
    // maximum, the number of keys in the source set
    char **dest_arr = malloc(sizeof(char *) * num_keys);
    if (dest_arr == NULL)
        return NULL;

    int dest_size = 0;

    if (read_tmpfile_strs(dest_arr, &dest_size, num_keys, path) != 0)
    {
        free_strarr(dest_arr, dest_size);
        return NULL;
    }

    struct Set *set = set_init(false, dest_size, dest_arr, true);

    free_strarr(dest_arr, dest_size);

    return set;
}

int read_tmpfile_strs(char **dest_arr, int *dest_size, unsigned int num_keys, char path[])
{
    char cur_str[PATH_MAX], *read_ptr = "";
    size_t i = 0;

    FILE *tmp_fptr = fopen(path, "r");
    if (tmp_fptr == NULL)
    {
        fprintf(stderr, "mmv: failed to open \"%s\" in \"r\" mode: %s\n", path, strerror(errno));
        return errno;
    }

    while (read_ptr != NULL && i < num_keys)
    {
        read_ptr = fgets(cur_str, PATH_MAX, tmp_fptr);

        if (read_ptr != NULL && strcmp(cur_str, "\n") != 0)
        {
            cur_str[strlen(cur_str) - 1] = '\0';

            cpy_str_to_arr(&dest_arr[(*dest_size)], cur_str);
            (*dest_size)++;

            i++;
        }
    }

    fclose(tmp_fptr);

    return 0;
}

void free_strarr(char **arr, int arr_size)
{
    for (int i = 0; i < arr_size; i++)
        free(arr[i]);

    free(arr);
}

int rename_paths(struct Set *src_set, struct Set *dest_set, struct Opts *opts)
{
    int *i, *j;
    char *src_str, *dest_str;

    for (i = set_begin(src_set), j = set_begin(dest_set); i < set_end(src_set) && j < set_end(dest_set);
         i = set_next(i), j = set_next(j))
    {
        src_str  = *get_set_pos(src_set, i);
        dest_str = *get_set_pos(dest_set, j);

        if (is_valid_key(j))
            rename_path(src_str, dest_str, opts);
    }

    return 0;
}

void rename_path(const char *src, const char *dest, struct Opts *opts)
{
    if (rename(src, dest) == -1)
    {
        fprintf(stderr, "mmv: \'%s\' to \'%s\': %s\n", src, dest, strerror(errno));

        if (errno == 2)
            remove(dest);
    }

    else if (opts->verbose)
        printf("  '%s' to '%s'\n", src, dest);
}

int rm_unedited_pairs(struct Set *src_set, struct Set *dest_set, struct Opts *opts)
{
    char *src_str, *dest_str;
    int *i, *j, *src_end_pos = set_end(src_set), *dest_end_pos = set_end(dest_set);

    for (i = set_begin(src_set), j = set_begin(dest_set); i < src_end_pos && j < dest_end_pos;
         i = set_next(i), j = set_next(j))
    {
        src_str  = *get_set_pos(src_set, i);
        dest_str = *get_set_pos(dest_set, j);

        if (strcmp(src_str, dest_str) == 0)
        {
            set_key(j, -1);

            if (opts->verbose)
                printf("  '%s' was not edited. No mv will be conducted.\n", src_str);
        }
    }

    return 0;
}

int rm_cycles(struct Set *src_set, struct Set *dest_set, struct Opts *opts)
{
    int is_dupe, *i, *j, *src_end_pos = set_end(src_set), *dest_end_pos = set_end(dest_set);
    unsigned long int u_key;
    char *dest_str, *tmp_path, **cur_src_pos;

    for (i = set_begin(src_set), j = set_begin(dest_set); i < src_end_pos && j < dest_end_pos;
         i = set_next(i), j = set_next(j))
    {
        if (is_valid_key(j))
        {
            dest_str = *get_set_pos(dest_set, j);
            u_key    = (unsigned int)*j;
            is_dupe  = is_duplicate_element(dest_str, src_set, &u_key);

            if (is_dupe == 0)
            {
                cur_src_pos             = get_set_pos(src_set, j);
                char template[]         = "_mmv_XXXXXX";
                char *tmp_path_parts[2] = {*cur_src_pos, template};

                tmp_path = strccat(tmp_path_parts, 2);
                if (tmp_path == NULL)
                {
                    perror("mmv: failed to allocate memory for cycle-removal temporary path");
                    return -1;
                }

                // create temporary name using the current name
                int tmp_fd = mkstemp(tmp_path);
                if (tmp_fd == -1)
                {
                    fprintf(stderr, "mmv: could not create temporary file \'%s\': %s\n", tmp_path, strerror(errno));
                    return -1;
                }

                // rename to temporary name
                rename_path(*cur_src_pos, tmp_path, opts);

                // update str in src map to temp_str
                free(*cur_src_pos);
                cpy_str_to_arr(cur_src_pos, tmp_path);
                free(tmp_path);
            }
        }
    }

    return 0;
}
