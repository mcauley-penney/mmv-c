/**
 * TODO:
 * 1. attempt to make collision detection less janky
 *      - if we can simplify it while retaining speed, we should
 *
 * 2. replace unsafe functions with safe ones
 *      - strncpy
 *      - see read_lines_from_fptr specifically
 *
 * 3. produce documentation
 *      - in .c for maintainers (that's me!), in .h for external users
 *
 * 4. determine what datatype to use for numerical values here
 *      - shouldn't use int for strlen?
 *
 * 5. make param names uniform, e.g. arg_count vs arr_len, etc.
 *
 * 6. enforce C naming conventions
 */

/**
 *  Title       : mmv.c
 *  Description : interactively move or rename files and directories
 *  Author      : Jacob Penney
 *  Credit      : itchyny/mmv
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

    // use variable (instead of, maybe, macro) because this string will be
    // modified in place and reused
    char tmp_path[] = "/tmp/mmv_XXXXXX";
    int hash, i, keyarr[argc], keyarr_len = 0, map_size = (6 * argc) + 1;
    struct StrPairNode *map[map_size];

    for (i = 0; i < map_size; i++)
        map[i] = NULL;

    for (i = 1; i < argc; i++)
    {
        char *cur_arg = argv[i];

        hash = attempt_strnode_map_insert(cur_arg, map, map_size);

        if (hash != -1)
        {
            keyarr[keyarr_len] = hash;
            keyarr_len++;
        }
    }

    write_old_names_to_tmp_file(tmp_path, map, keyarr, keyarr_len);

    open_tmp_file_in_editor(tmp_path);

    read_new_names_from_tmp_file(tmp_path, map, keyarr, keyarr_len);

    rm_path(tmp_path);

    rename_files(map, map_size, keyarr, keyarr_len);

    free_map(map, keyarr, keyarr_len);

    // ------------------------------------------------------------------------
#ifdef TESTING
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("time: %f\n", cpu_time_used);
#endif
    // ------------------------------------------------------------------------

    return EXIT_SUCCESS;
}

struct StrPairNode *add_strpair_node(struct StrPairNode *cur_node, char *src)
{
    if (cur_node == NULL)
    {
        cur_node = init_pair_node(src);
        return cur_node;
    }

    if (strcmp(cur_node->src, src) == 0)
        return NULL;

    cur_node->next = add_strpair_node(cur_node->next, src);

    return cur_node;
}

int attempt_strnode_map_insert(char *str, struct StrPairNode *map[], int map_size)
{
    int hash = get_fnv_32a_str_hash(str, map_size);

    hash = hashmap_insert(map, str, hash);

    return hash;
}

Fnv32_t get_fnv_32a_str_hash(char *str, int map_size)
{
    Fnv32_t hval = ((Fnv32_t)0x811c9dc5);
    unsigned char *s = (unsigned char *)str; /* unsigned string */

    // FNV-1a hash each octet in the buffer
    while (*s)
    {
        /* xor the bottom with the current octet */
        hval ^= (Fnv32_t)*s++;

        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
    }

    return hval % map_size;
}

void free_map(struct StrPairNode *map[], const int keyarr[], const int keyarr_len)
{
    int i;
    struct StrPairNode *wkg_node;

    for (i = 0; i < keyarr_len; i++)
    {
        wkg_node = map[keyarr[i]];
        free_pair_ll(wkg_node);
    }
}

void free_pair_ll(struct StrPairNode *node)
{
    if (node != NULL)
    {
        free_pair_ll(node->next);

        free(node->src);
        free(node->dest);
        free(node);
    }
}

FILE *get_tmp_path_fptr(char *tmp_path)
{
    FILE *fptr;
    int tmp_fd = mkstemp(tmp_path);

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

int hashmap_insert(struct StrPairNode *map[], char *str, int hash)
{
    int output = map[hash] == NULL ? hash : -1;

    map[hash] = add_strpair_node(map[hash], str);

    return output;
}

struct StrPairNode *init_pair_node(char *src_str)
{
    int src_len = (strlen(src_str) + 1);

    struct StrPairNode *new_node = malloc(sizeof(struct StrPairNode));
    new_node->next = NULL;
    new_node->dest = malloc(src_len * sizeof(char));
    new_node->src = malloc(src_len * sizeof(char));

    // init dest to same string so that rename may ignore unchanged names
    strcpy(new_node->dest, src_str);
    strcpy(new_node->src, src_str);

    return new_node;
}

int map_find_src_pos(struct StrPairNode *map[], int hash, char *str)
{
    int i;
    struct StrPairNode *wkg_node = map[hash];

    // access map at key
    for (i = 0; wkg_node != NULL; i++)
    {
        if (strcmp(str, wkg_node->src) == 0)
            return i;

        wkg_node = wkg_node->next;
    }

    return -1;
}

void map_update_src(struct StrPairNode *map[], int hash, int pos, char *new_str)
{
    int i;
    struct StrPairNode *wkg_node = map[hash];

    for (i = 0; i < pos; i++)
        wkg_node = wkg_node->next;

    free(wkg_node->src);

    wkg_node->src = malloc((strlen(new_str) + 1) * sizeof(wkg_node->src));

    strcpy(wkg_node->src, new_str);
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

void open_tmp_file_in_editor(const char *path)
{
    char *editor_cmd = "$EDITOR ";
    int edit_cmd_len = strlen(editor_cmd) + strlen(path) + 1;
    char *edit_cmd = malloc(edit_cmd_len * sizeof(edit_cmd));

    strcpy(edit_cmd, editor_cmd);
    strcat(edit_cmd, path);

    // open temporary file containing argv using editor of choice
    system(edit_cmd);

    free(edit_cmd);
}

void read_lines_from_fptr(FILE *fptr, struct StrPairNode *map[], const int keyarr[], const int keyarr_len)
{
    // TODO: must do more to protect against buffer overflow
    const int max_str_len = 500;
    char cur_str[max_str_len], *read_ptr = "";
    int cur_key, i = 0;

    while (read_ptr != NULL && i < keyarr_len)
    {
        read_ptr = fgets(cur_str, max_str_len, fptr);

        if (read_ptr != NULL && strcmp(cur_str, "\n") != 0)
        {
            cur_key = keyarr[i];

            free(map[cur_key]->dest);
            map[cur_key]->dest = malloc(max_str_len * sizeof(char));

            rm_str_nl(cur_str);
            strcpy(map[cur_key]->dest, cur_str);

            i++;
        }
    }
}

void read_new_names_from_tmp_file(char tmp_path[], struct StrPairNode *map[], int keyarr[], int keyarr_len)
{
    FILE *tmp_fptr;

    open_file(tmp_path, "r", &tmp_fptr);
    read_lines_from_fptr(tmp_fptr, map, keyarr, keyarr_len);
    fclose(tmp_fptr);
}

void rename_files(struct StrPairNode *map[], const int map_size, const int keyarr[], const int keyarr_len)
{
    int i;

    for (i = 0; i < keyarr_len; i++)
    {
        struct StrPairNode *wkg_node = map[keyarr[i]];

        while (wkg_node != NULL)
        {
            char *cur_dest = wkg_node->dest;

            // if current destination exists in fs
            if (access(cur_dest, F_OK) == 0)
            {
                // TODO:
                //  1. attempt to make this smarter?
                //  2. make this it's own function?

                // check if the dest is in our map as a source
                int dest_hash = get_fnv_32a_str_hash(cur_dest, map_size);
                int node_pos = map_find_src_pos(map, dest_hash, cur_dest);

                if (node_pos != -1)
                {
                    char tmp_path[] = "mmv_XXXXXX";
                    int tmp_fd = mkstemp(tmp_path);
                    close(tmp_fd);

                    map_update_src(map, dest_hash, node_pos, tmp_path);
                    rename_path_pair(cur_dest, tmp_path);
                }
            }

            rename_path_pair(wkg_node->src, cur_dest);

            wkg_node = wkg_node->next;
        }
    }
}

void rename_path_pair(char *src, char *dest)
{
    int rename_result = rename(src, dest);

    if (rename_result == -1)
        fprintf(stderr, "ERROR: Could not rename \"%s\" to \"%s\"\n", src, dest);
}

void rm_path(char *path)
{
    int rm_success = remove(path);

    if (rm_success == -1)
        fprintf(stderr, "ERROR: Unable to delete \"%s\"\n", path);
}

// https://stackoverflow.com/a/42564670
void rm_str_nl(char *str)
{
    char *end = str + strlen(str) - 1;

    while (end > str && isspace(*end))
        end--;

    *(end + 1) = '\0';
}

void write_map_to_fptr(FILE *fptr, struct StrPairNode *map[], int keys[], const int num_keys)
{
    int i;
    struct StrPairNode *wkg_node;

    for (i = 0; i < num_keys; i++)
    {
        wkg_node = map[keys[i]];

        while (wkg_node != NULL)
        {
            fprintf(fptr, "%s\n", wkg_node->src);
            wkg_node = wkg_node->next;
        }
    }
}

void write_old_names_to_tmp_file(char tmp_path[], struct StrPairNode *map[], int keyarr[], int keyarr_len)
{
    FILE *tmp_fptr = get_tmp_path_fptr(tmp_path);
    write_map_to_fptr(tmp_fptr, map, keyarr, keyarr_len);
    // there is no corresponding explicit fopen() call for this
    // fclose() because mkstemp in get_tmp_path_fptr() opens
    // temp file for us
    fclose(tmp_fptr);
}
