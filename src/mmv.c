/**
 *  Title       : mmv-c
 *  Description : interactively move files and directories
 *  Author      : Jacob M. Penney
 */

#include "mmv.h"

int main(int argc, char *argv[])
{
    // ------------------------------------------------------------------------
#ifdef TESTING
    clock_t start, end;
    double cpu_time_used;
    start = clock();
#endif
    // ------------------------------------------------------------------------

    sanitize_num_args(&argc);

    struct MapKeyArr *keys = NULL;
    char **map = NULL;

    make_argv_hashmap(argv, argc, &map, &keys);

    // disallow anyone but user from accessing created files
    char tmp_path[] = "/tmp/mmv_XXXXXX";
    umask(077);

    write_old_names_to_tmp_file(tmp_path, map, keys);

    open_tmp_file_in_editor(tmp_path);

    rename_filesystem_items(tmp_path, map, keys);

    rm_path(tmp_path);
    free_map_nodes(map, keys);
    free(map);
    free(keys);

// ------------------------------------------------------------------------
#ifdef TESTING
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("cpu time: %f\n", cpu_time_used);
#endif
    // ------------------------------------------------------------------------

    return EXIT_SUCCESS;
}

unsigned int get_fnv_32a_str_hash(char *str, const unsigned int map_size)
{
    unsigned char *s = (unsigned char *)str; /* unsigned string */
    Fnv32_t hval = ((Fnv32_t)0x811c9dc5);

    // FNV-1a hash each octet in the buffer
    while (*s)
    {
        // xor the bottom with the current octet
        hval ^= (Fnv32_t)*s++;

        // multiply by the 32 bit FNV magic prime mod 2^32
        hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
    }

    return hval % map_size;
}

void free_map_nodes(char *map[], struct MapKeyArr *keys)
{
    size_t i;
    for (i = 0; i < keys->num_keys; i++)
        free(map[keys->keyarr[i]]);
}

FILE *__attribute__((malloc)) get_tmp_path_fptr(char *tmp_path)
{
    FILE *fptr;
    int tmp_fd = mkstemp(tmp_path);

    if (tmp_fd == -1)
    {
        fprintf(stderr, "mmv: failed to open \"%s\" as file descriptor: %s\n", tmp_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fptr = fdopen(tmp_fd, "w");

    if (fptr == NULL)
    {
        fprintf(stderr, "mmv: failed to open \"%s\" as file pointer: %s\n", tmp_path, strerror(errno));
        rm_path(tmp_path);
        exit(EXIT_FAILURE);
    }

    return fptr;
}

void init_hashmap_objs(const unsigned int num_args, const unsigned int map_size, char ***map, struct MapKeyArr **keys)
{
    unsigned int i;

    // struct with FAM must be malloced and size of FAM must be included
    *keys = malloc(sizeof(struct MapKeyArr) + (num_args - 1) * sizeof(int));
    if ((*keys) == NULL)
    {
        perror("mmv: failed to allocate memory for hash keys struct: ");
        exit(EXIT_FAILURE);
    }

    // calloc for hashmap array because we would like to be able to
    // dynamically assign array size but avoid VLA. Calloc is superior
    // to malloc for this application.
    *map = calloc(map_size, sizeof(char *));
    if ((*map) == NULL)
    {
        perror("mmv: failed to allocate memory for argv hash map: ");
        exit(EXIT_FAILURE);
    }

    (*keys)->num_keys = 0;

    for (i = 0; i < map_size; i++)
        (*map)[i] = NULL;
}

void init_node_src(char **array_pos, const char *src_str)
{
    *array_pos = malloc((strlen(src_str) + 1) * sizeof(char));

    if (array_pos == NULL)
    {
        perror("mmv: failed to allocate memory for new map node source str: ");
        exit(EXIT_FAILURE);
    }

    strcpy(*array_pos, src_str);
}

void make_argv_hashmap(char *argv[], int argc, char ***map, struct MapKeyArr **keys)
{
    const unsigned int u_argc = (unsigned int)argc;
    const unsigned int map_size = (6 * (u_argc - 1)) + 1;

    init_hashmap_objs(u_argc, map_size, map, keys);

    char *cur_str = NULL;
    int dupe_found;
    unsigned int i, hash;

    // start at 1 instead of 0 to avoid reading program invocation
    for (i = 1; i < u_argc; i++)
    {
        cur_str = argv[i];
        hash = get_fnv_32a_str_hash(cur_str, map_size);

        dupe_found = probe_for_null_hashpos(map, map_size, cur_str, &hash);

        if (dupe_found != 0)
        {
            init_node_src(&(*map)[hash], cur_str);
            (*keys)->keyarr[(*keys)->num_keys] = hash;
            (*keys)->num_keys++;
        }
    }
}

void open_file(char *path, const char *mode, FILE **fptr)
{
    *fptr = fopen(path, mode);

    if (fptr == NULL)
    {
        fprintf(stderr, "mmv: failed to open \"%s\" in \"%s\" mode: %s\n", path, mode, strerror(errno));
        rm_path(path);
        exit(EXIT_FAILURE);
    }
}

void open_tmp_file_in_editor(const char *path)
{
    char *editor_name = getenv("EDITOR");

    if (editor_name == NULL)
        editor_name = "nano";

    const size_t cmd_len = strlen(editor_name) + 1 + strlen(path) + 1;
    char *edit_cmd = malloc(cmd_len * sizeof(edit_cmd));
    if (edit_cmd == NULL)
    {
        perror("mmv: failed to allocate memory for $EDITOR command: ");
        exit(EXIT_FAILURE);
    }

    snprintf(edit_cmd, cmd_len, "%s %s", editor_name, path);

    // open temporary file containing argv using editor of choice
    int cmd_ret = system(edit_cmd);

    if (cmd_ret != 0)
        fprintf(stderr, "mmv: \'%s\' returned non-zero exit status: %d\n", editor_name, cmd_ret);

    free(edit_cmd);
}

int probe_for_null_hashpos(char ***map, const unsigned int map_size, const char *cur_str, unsigned int *hash)
{
    char **deref_map = *map;
    int dupe_found = -1;
    unsigned int hash_cp = *hash;

    while (deref_map[hash_cp] != NULL && dupe_found != 0)
    {
        dupe_found = strcmp(deref_map[hash_cp], cur_str);

        if (hash_cp + 1 == map_size)
            hash_cp = 0;
        else
            hash_cp++;
    }

    *hash = hash_cp;

    return dupe_found;
}

void rename_filesystem_items(char tmp_path[], char *map[], struct MapKeyArr *keys)
{
    char cur_str[NAME_MAX], *read_ptr = "";
    FILE *tmp_fptr;
    unsigned int *keyarr = keys->keyarr;
    size_t i = 0;

    open_file(tmp_path, "r", &tmp_fptr);

    while (read_ptr != NULL && i < keys->num_keys)
    {
        read_ptr = fgets(cur_str, NAME_MAX, tmp_fptr);

        // only proceed with iteration if the current string
        // isn't just a newline
        if (read_ptr != NULL && strcmp(cur_str, "\n") != 0)
        {
            // fgets is guaranteed to return a string with a
            // newline. Replace it with null byte
            cur_str[strlen(cur_str) - 1] = '\0';
            rename_path_pair(map[keyarr[i]], cur_str);
            i++;
        }
    }

    fclose(tmp_fptr);
}

void rename_path_pair(const char *src, const char *dest)
{
    int rename_result = rename(src, dest);

    if (rename_result == -1)
        fprintf(stderr, "mmv: \'%s\': %s\n", src, strerror(errno));
}

void rm_path(char *path)
{
    int rm_success = remove(path);

    if (rm_success == -1)
        fprintf(stderr, "mmv: failed to delete \"%s\": %s\n", path, strerror(errno));
}

void sanitize_num_args(int *args_len)
{
    if (*args_len < 2)
    {
        fprintf(stderr, "mmv: missing file operand\n");
        exit(EXIT_FAILURE);
    }

    else if (*args_len > ARG_MAX)
    {
        *args_len = ARG_MAX;
        fprintf(stderr, "mmv: only modifying first 10,000 operands\n");
    }
}

void write_old_names_to_tmp_file(char path[], char *map[], struct MapKeyArr *keys)
{
    FILE *tmp_fptr;
    size_t i;

    tmp_fptr = get_tmp_path_fptr(path);

    for (i = 0; i < keys->num_keys; i++)
        fprintf(tmp_fptr, "%s\n", map[keys->keyarr[i]]);

    // there is no corresponding explicit fopen() call for this
    // fclose() because mkstemp in get_tmp_path_fptr() opens
    // temp file for us
    fclose(tmp_fptr);
}
