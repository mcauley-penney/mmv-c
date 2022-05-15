/**
 *  Title       : mmv.c
 *  Description : interactively move or rename files and directories
 *  Author      : JMP
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

    if (argc < 2)
    {
        fprintf(stderr, "mmv: missing file operand\n");
        exit(EXIT_FAILURE);
    }

    char tmp_path[] = "/tmp/mmv_XXXXXX";
    int hash;
    unsigned int i;
    const unsigned int u_argc = (unsigned int)argc;
    const unsigned int map_size = (6 * (u_argc - 1)) + 1;
    struct MapKeyArr *keys = malloc(sizeof(struct MapKeyArr) + ((u_argc - 1) * sizeof(int)));
    struct StrPairNode **map = malloc(map_size * sizeof(struct StrPairNode));

    keys->num_keys = 0;

    for (i = 0; i < map_size; i++)
        map[i] = NULL;

    // start at 1 instead of 0 to avoid reading program invocation
    for (i = 1; i < u_argc; i++)
    {
        hash = attempt_strnode_map_insert(argv[i], map, map_size);

        if (hash != -1)
        {
            keys->keyarr[keys->num_keys] = hash;
            (keys->num_keys)++;
        }
    }

    // disallow anyone but user from accessing created files
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

/**
 * When given a source string, this function will recursively
 * attempt to place that string in a newly-created node at the
 * end of the linked list of nodes attached to the given node
 * parameter.
 *
 * If the current node is NULL, a new node is created and the
 * string is placed as the source member of that node. If,
 * during the process of searching for the end of the linked
 * list, a node with the same source string is found, the
 * function simply "reattaches" the current node and exits.
 * This disallows nodes with the same source string in the
 * map, eliminating populating the temporary renaming buffer
 * with duplicate items. For example, if the user passes the
 * same file name to mmv multiple times, this will stop those
 * duplicates from propagating into the temporary renaming
 * buffer.
 */
struct StrPairNode *add_strpair_node(struct StrPairNode *cur_node, const char *new_src)
{
    // if current node is null, insert new node at current position
    // by returning the newly created node as node->next
    if (cur_node == NULL)
        return init_pair_node(new_src);

    // if the str at the current node matches the new str, do nothing
    if (strcmp(cur_node->src, new_src) == 0)
        return cur_node;

    cur_node->next = add_strpair_node(cur_node->next, new_src);

    return cur_node;
}

int attempt_strnode_map_insert(char *str, struct StrPairNode *map[], unsigned int map_size)
{
    int hash = get_fnv_32a_str_hash(str, map_size);

    // Don't add hash to list of keys if already present.
    // Because we use chaining in each hash, we only need to
    // have a list of keys indicating the root of each chain
    int hash_to_insert = map[hash] == NULL ? hash : -1;

    map[hash] = add_strpair_node(map[hash], str);

    return hash_to_insert;
}

int get_fnv_32a_str_hash(char *str, unsigned int map_size)
{
    unsigned char *s = (unsigned char *)str; /* unsigned string */
    Fnv32_t hval = ((Fnv32_t)0x811c9dc5), unsigned_hash;

    // FNV-1a hash each octet in the buffer
    while (*s)
    {
        // xor the bottom with the current octet
        hval ^= (Fnv32_t)*s++;

        // multiply by the 32 bit FNV magic prime mod 2^32
        hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
    }

    // map size is a signed int guaranteed to be positive. We
    // want our hval, an unsigned int, to become a positive
    // value within the range of an int. We can safely cast
    // our map size to an unsigned int and perform modulo,
    // granting us an unsigned int in the range of int
    unsigned_hash = hval % map_size;

    return (int)unsigned_hash;
}

void free_map_nodes(struct StrPairNode *map[], struct MapKeyArr *keys)
{
    int i;

    // for each position in the hashmap array that has a node,
    // recursively free all nodes connected to it
    for (i = 0; i < keys->num_keys; i++)
        free_pair_ll(map[keys->keyarr[i]]);
}

void free_pair_ll(struct StrPairNode *node)
{
    if (node != NULL)
    {
        free_pair_ll(node->next);

        free(node->src);
        free(node);
    }
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

struct StrPairNode *init_pair_node(const char *src_str)
{
    struct StrPairNode *new_node = malloc(sizeof(struct StrPairNode));
    if (new_node == NULL)
    {
        perror("mmv: failed to allocate memory for new map node: ");
        exit(EXIT_FAILURE);
    }

    new_node->src = malloc((strlen(src_str) + 1) * sizeof(char));
    if (new_node->src == NULL)
    {
        perror("mmv: failed to allocate memory for new map node source str: ");
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->src, src_str);
    new_node->next = NULL;

    return new_node;
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
    system(edit_cmd);

    free(edit_cmd);
}

void rename_filesystem_items(char tmp_path[], struct StrPairNode *map[], struct MapKeyArr *keys)
{
    char cur_str[NAME_MAX], *read_ptr = "";
    FILE *tmp_fptr;
    int i = 0, *keyarr = keys->keyarr;
    struct StrPairNode *wkg_node = map[keyarr[i]];

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

            rename_path_pair(wkg_node->src, cur_str);

            wkg_node = wkg_node->next;

            if (wkg_node == NULL && i + 1 < keys->num_keys)
            {
                i++;
                wkg_node = map[keyarr[i]];
            }
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

void write_map_to_fptr(FILE *fptr, struct StrPairNode *map[], struct MapKeyArr *keys)
{
    int i;
    struct StrPairNode *wkg_node;

    for (i = 0; i < keys->num_keys; i++)
    {
        wkg_node = map[keys->keyarr[i]];

        while (wkg_node != NULL)
        {
            fprintf(fptr, "%s\n", wkg_node->src);
            wkg_node = wkg_node->next;
        }
    }
}

void write_old_names_to_tmp_file(char path[], struct StrPairNode *map[], struct MapKeyArr *keys)
{
    FILE *tmp_fptr;

    tmp_fptr = get_tmp_path_fptr(path);

    write_map_to_fptr(tmp_fptr, map, keys);

    // there is no corresponding explicit fopen() call for this
    // fclose() because mkstemp in get_tmp_path_fptr() opens
    // temp file for us
    fclose(tmp_fptr);
}
