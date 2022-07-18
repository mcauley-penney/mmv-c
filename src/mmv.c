// TODO:
//  1. try and update map and keys pointers to drop one star
//  2. feat: ensure directory-moving ability

/**
 *  Title       : mmv-c
 *  Description : interactively move files and directories
 *  Author      : Jacob M. Penney
 */

#include "mmv.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "mmv: missing file operand\n");
        exit(EXIT_FAILURE);
    }

    char **map = NULL;
    struct MapKeyArr *keys = NULL;

    make_argv_hashmap(argv, argc, &map, &keys);

    char tmp_path[] = "/tmp/mmv_XXXXXX";

    // even though all files created by mmv will be removed
    // disallow anyone but user from accessing them.
    umask(077);

    write_old_names_to_tmp_file(tmp_path, map, keys);

    open_tmp_file_in_editor(tmp_path);

    rename_filesystem_items(tmp_path, map, keys);

    rm_path(tmp_path);
    free_map_nodes(map, keys);
    free(map);
    free(keys);

    return EXIT_SUCCESS;
}

unsigned int calc_fnv32a_str_hash(char *str, const unsigned int map_size)
{
    unsigned char *s = (unsigned char *)str;
    Fnv32_t hval = ((Fnv32_t)0x811c9dc5);

    // FNV-1a hash each octet in the buffer
    while (*s)
    {
        hval ^= (Fnv32_t)*s++;
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

FILE *__attribute__((malloc)) open_tmp_path_fptr(char *tmp_path)
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
    if (keys == NULL)
    {
        perror("mmv: failed to allocate memory for hash keys struct: ");
        exit(EXIT_FAILURE);
    }

    *map = malloc(sizeof(char *) * map_size);
    if (map == NULL)
    {
        perror("mmv: failed to allocate memory for argv hash map: ");
        exit(EXIT_FAILURE);
    }

    (*keys)->num_keys = 0;

    for (i = 0; i < map_size; i++)
        (*map)[i] = NULL;
}

void cp_str_to_arr(char **array_pos, const char *src_str)
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
    char *cur_str = NULL;
    int dupe_found;
    unsigned int hash, i;
    const unsigned int u_argc = (unsigned int)argc;

    // (6 * (val - 1)) + 1 is a formula that creates a
    // prime number using some value as a base. We are
    // using this to create a prime hash map size to
    // attempt to mitigate collisions. While the FNV
    // hash is known to be of high quality and this is
    // supposed to not be necessary, this is not heavy
    // to implement and adds a layer of protection.
    const unsigned int map_size = (6 * (u_argc - 1)) + 1;

    init_hashmap_objs(u_argc, map_size, map, keys);

    // start at 1 instead of 0 to avoid reading program invocation
    for (i = 1; i < u_argc; i++)
    {
        cur_str = argv[i];
        hash = calc_fnv32a_str_hash(cur_str, map_size);

        dupe_found = probe_for_null_hashpos(map, map_size, cur_str, &hash);

        if (dupe_found != 0)
        {
            cp_str_to_arr(&(*map)[hash], cur_str);
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
    int ret;

    char *editor_name = getenv("EDITOR");

    if (editor_name == NULL)
        editor_name = "nano";

    // provide space for "$EDITOR path\0", e.g. "nano file.txt\0"
    const size_t cmd_len = strlen(editor_name) + strlen(path) + 2;
    char *edit_cmd = malloc(sizeof(edit_cmd) * cmd_len);
    if (edit_cmd == NULL)
    {
        perror("mmv: failed to allocate memory for $EDITOR command string: ");
        exit(EXIT_FAILURE);
    }

    ret = snprintf(edit_cmd, cmd_len, "%s %s", editor_name, path);

    if (ret < 0 || ret > cmd_len)
        perror("mmv: couldn't create $EDITOR command string: ");

    // open temporary file containing argv using editor of choice
    ret = system(edit_cmd);

    if (ret != 0)
        fprintf(stderr, "mmv: \'%s\' returned non-zero exit status: %d\n", editor_name, ret);

    free(edit_cmd);
}

int probe_for_null_hashpos(char ***map, const unsigned int map_size, const char *cur_str, unsigned int *hash)
{
    char **deref_map = *map;
    int dupe_found = -1;

    while (deref_map[*hash] != NULL && dupe_found != 0)
    {
        dupe_found = strcmp(deref_map[*hash], cur_str);

        *hash = (*hash + 1 != map_size) ? *hash + 1 : 0;
    }

    return dupe_found;
}

void rename_filesystem_items(char tmp_path[], char *map[], struct MapKeyArr *keys)
{
    char cur_str[PATH_MAX], *read_ptr = "";
    FILE *tmp_fptr;
    unsigned int *keyarr = keys->keyarr;
    size_t i = 0;

    open_file(tmp_path, "r", &tmp_fptr);

    while (read_ptr != NULL && i < keys->num_keys)
    {
        read_ptr = fgets(cur_str, PATH_MAX, tmp_fptr);

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

// TODO:
// must investigate directory moving and renaming functionality.
// We think this doesn't take care of all that, so we should
// make this a more full wrapper and include all that
void rename_path_pair(const char *src, const char *dest)
{
    int rename_result = rename(src, dest);

    if (rename_result == -1)
        fprintf(stderr, "mmv: \'%s\' to \'%s\': %s\n", src, dest, strerror(errno));
}

void rm_path(char *path)
{
    if (remove(path) == -1)
        fprintf(stderr, "mmv: failed to delete \'%s\': %s\n", path, strerror(errno));
}

void write_old_names_to_tmp_file(char path[], char *map[], struct MapKeyArr *keys)
{
    size_t i;

    FILE *tmp_fptr = open_tmp_path_fptr(path);

    for (i = 0; i < keys->num_keys; i++)
        fprintf(tmp_fptr, "%s\n", map[keys->keyarr[i]]);

    fclose(tmp_fptr);
}
