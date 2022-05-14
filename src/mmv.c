/**
 *  Title       : mmv.c
 *  Description : interactively move or rename files and directories
 *  Author      : JMP
 */

#include "mmv.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "mmv: missing file operand\n");
        exit(EXIT_FAILURE);
    }

    char tmp_path[] = "/tmp/mmv_XXXXXX";
    int hash, i;
    const int map_size = (6 * (argc - 1)) + 1;
    struct MapKeyArr *keys = malloc(sizeof(struct MapKeyArr) + ((unsigned int)(argc - 1) * sizeof(int)));
    struct StrPairNode **map = malloc((unsigned int)map_size * sizeof(struct StrPairNode));

    keys->num_keys = 0;

    for (i = 0; i < map_size; i++)
        map[i] = NULL;

    // start at 1 instead of 0 to avoid reading program invocation
    for (i = 1; i < argc; i++)
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

    read_new_names_from_tmp_file(tmp_path, map, keys);

    rm_path(tmp_path);

    rename_files(map, keys);

    free_map_nodes(map, keys);
    free(map);
    free(keys);

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

int attempt_strnode_map_insert(char *str, struct StrPairNode *map[], int map_size)
{
    int hash = get_fnv_32a_str_hash(str, map_size);

    // if hash is already in list of keys, don't add it to list of
    // keys. Because we use chaining in each hash, we only need to
    // have a list of keys indicating the root of each chain
    int hash_to_insert = map[hash] == NULL ? hash : -1;

    map[hash] = add_strpair_node(map[hash], str);

    return hash_to_insert;
}

int get_fnv_32a_str_hash(char *str, int map_size)
{
    Fnv32_t hval = ((Fnv32_t)0x811c9dc5), unsigned_hash;
    ;
    unsigned char *s = (unsigned char *)str; /* unsigned string */

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
    unsigned_hash = hval % (Fnv32_t)map_size;

    return (int)unsigned_hash;
}

void free_map(struct StrPairNode *map[], struct MapKeyArr *keys)
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
        free(node->dest);
        free(node);
    }
}

FILE *__attribute__((malloc)) get_tmp_path_fptr(char *tmp_path)
{
    FILE *fptr;
    int tmp_fd = mkstemp(tmp_path);

    if (tmp_fd == -1)
    {
        fprintf(stderr, "mmv: failed to open \"%s\" as file descriptor\n", tmp_path);
        exit(EXIT_FAILURE);
    }

    fptr = fdopen(tmp_fd, "w");

    if (fptr == NULL)
    {
        fprintf(stderr, "mmv: failed to open \"%s\" as file pointer\n", tmp_path);
        rm_path(tmp_path);
        exit(EXIT_FAILURE);
    }

    return fptr;
}

struct StrPairNode *init_pair_node(const char *src_str)
{
    const size_t src_len = (strlen(src_str) + 1) * sizeof(char);

    struct StrPairNode *new_node = malloc(sizeof(struct StrPairNode));
    if (new_node == NULL)
    {
        fprintf(stderr, "mmv: failed to allocate memory for new map node\n");
        exit(EXIT_FAILURE);
    }

    new_node->src = malloc(src_len);
    if (new_node->src == NULL)
    {
        fprintf(stderr, "mmv: failed to allocate memory for new map node source\n");
        exit(EXIT_FAILURE);
    }

    new_node->dest = malloc(src_len);
    if (new_node->dest == NULL)
    {
        fprintf(stderr, "mmv: failed to allocate memory for new map node destination\n");
        exit(EXIT_FAILURE);
    }

    new_node->next = NULL;

    // init dest to same string so that rename may ignore unchanged names
    strcpy(new_node->src, src_str);
    strcpy(new_node->dest, src_str);

    return new_node;
}

void open_file(char *path, const char *mode, FILE **fptr)
{
    *fptr = fopen(path, mode);

    if (fptr == NULL)
    {
        fprintf(stderr, "mmv: failed to open \"%s\" in \"%s\" mode\n", path, mode);
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

    snprintf(edit_cmd, cmd_len, "%s %s", editor_name, path);

    // open temporary file containing argv using editor of choice
    system(edit_cmd);

    free(edit_cmd);
}

void read_new_names_from_tmp_file(char tmp_path[], struct StrPairNode *map[], struct MapKeyArr *keys)
{
    char cur_str[NAME_MAX], *read_ptr = "";
    FILE *tmp_fptr;
    int cur_key, i = 0;

    open_file(tmp_path, "r", &tmp_fptr);

    while (read_ptr != NULL && i < keys->num_keys)
    {
        read_ptr = fgets(cur_str, NAME_MAX, tmp_fptr);

        if (read_ptr != NULL && strcmp(cur_str, "\n") != 0)
        {
            // replace newline with null byte
            cur_str[strlen(cur_str) - 1] = '\0';

            cur_key = keys->keyarr[i];
            free(map[cur_key]->dest);

            map[cur_key]->dest = malloc((strlen(cur_str) + 1) * sizeof(map[cur_key]->dest));
            if (map[cur_key]->dest == NULL)
            {
                fprintf(stderr, "mmv: failed to allocate memory for given destination \"%s\"\n", cur_str);
                rm_path(tmp_path);
                exit(EXIT_FAILURE);
            }

            strcpy(map[cur_key]->dest, cur_str);

            i++;
        }
    }

    fclose(tmp_fptr);
}

void rename_files(struct StrPairNode *map[], struct MapKeyArr *keys)
{
    int i;

    for (i = 0; i < keys->num_keys; i++)
    {
        struct StrPairNode *wkg_node = map[keys->keyarr[i]];

        while (wkg_node != NULL)
        {
            rename_path_pair(wkg_node->src, wkg_node->dest);

            wkg_node = wkg_node->next;
        }
    }
}

void rename_path_pair(const char *src, const char *dest)
{
    int rename_result = rename(src, dest);

    if (rename_result == -1)
        fprintf(stderr, "mmv: failed to rename \"%s\" to \"%s\"\n", src, dest);
}

void rm_path(char *path)
{
    int rm_success = remove(path);

    if (rm_success == -1)
        fprintf(stderr, "mmv: failed to delete \"%s\"\n", path);
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
