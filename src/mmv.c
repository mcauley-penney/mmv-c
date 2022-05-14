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
    struct StrPairNode *map[map_size];

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

    free_map(map, keys);

    free(keys);

    return EXIT_SUCCESS;
}

/**
 * @brief recursively add or reject a node from linked list of string
 * pair nodes
 *
 * @param cur_node: working node in iteration across linked list
 * @param new_src: source string (old name) to give to new node; will
 *      be rejected if a node is found to already contain it
 *
 * @return NULL or new node; if new node, new node is appended to end
 *      of linked list
 */
struct StrPairNode *add_strpair_node(struct StrPairNode *cur_node, const char *new_src)
{
    // if current node is null, insert new node at current position
    if (cur_node == NULL)
        return init_pair_node(new_src);

    // if the str at the current node matches the new str, do nothing
    if (strcmp(cur_node->src, new_src) == 0)
        return cur_node;

    cur_node->next = add_strpair_node(cur_node->next, new_src);

    return cur_node;
}

/**
 * @brief attempt to add a source string to hashmap of string pair
 *      nodes as a new node
 *
 * @param str: string to hash and add to map
 * @param map: map of string pair nodes to add string to
 * @param map_size: current size of the map
 *
 * @return hash where new node was inserted; -1 if key already exists
 */
int attempt_strnode_map_insert(char *str, struct StrPairNode *map[], int map_size)
{
    int hash = get_fnv_32a_str_hash(str, map_size);

    // if hash is already in list of keys, don't add it to list of
    // keys. Because we use chaining in each hash, we only need to
    // have a list of keys indicating the root of each chain
    int hash_to_insert = map[hash] == NULL ? hash : -1;

    // attempt to add node. Will return NULL if duplicate str is
    // found
    map[hash] = add_strpair_node(map[hash], str);

    return hash_to_insert;
}

/**
 * @brief hash a string with Fowler–Noll–Vo 1a 32bit hash
 *
 * @param str: string to hash
 * @param map_size: size of map; used to modulo the hash to fit into array
 *
 * @return hash after modulo
 */
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

/**
 * @brief free map of string pair nodes
 *
 * @param map: map to free
 * @param keys: struct containing list of keys where nodes exist
 */
void free_map(struct StrPairNode *map[], struct MapKeyArr *keys)
{
    int i;
    for (i = 0; i < keys->num_keys; i++)
        free_pair_ll(map[keys->keyarr[i]]);
}

/**
 * @brief recursively free all nodes in a linked list
 *
 * @param node: working node in recursion across linked list
 */
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
/**
 * @brief open given path and get its file pointer
 *
 * @param tmp_path: string of file path to open
 *
 * @return file pointer to opened path
 */
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

/**
 * @brief initialize and return a string pair node
 *
 * @param src_str: string to give to node as source (old name) and
 *      temporarily as destination (new name)
 *
 * @return initialized node
 */
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

/**
 * @brief open a file, given a mode and path to the file
 *
 * @param path: file path to open
 * @param mode: mode to open file in
 * @param **fptr: out param, holds opened file
 */
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

/**
 * @brief get the user's $EDITOR env variable and open temp file with it
 *
 * @param path: file path to open in editor
 */
void open_tmp_file_in_editor(const char *path)
{
    char *editor_name = getenv("EDITOR");

    if (editor_name == NULL)
        editor_name = "nano";

    const size_t cmd_len = strlen(editor_name) + 1 + strlen(path) + 1;
    char edit_cmd[cmd_len];

    snprintf(edit_cmd, cmd_len, "%s %s", editor_name, path);

    // open temporary file containing argv using editor of choice
    system(edit_cmd);
}

/**
 * @brief read lines out of a file path and store them in hashmap
 *      of string pair nodes as destination (new name) strings
 *
 * @param tmp_path: path to open and read lines from
 * @param map: map to store retrieved strings in
 * @param keys: struct containing list of keys of locations of
 *      string pair nodes in hashmap
 */
void read_new_names_from_tmp_file(char tmp_path[], struct StrPairNode *map[], struct MapKeyArr *keys)
{
    const unsigned int max_str_len = 500;
    char cur_str[max_str_len], *read_ptr = "";
    FILE *tmp_fptr;
    int cur_key, i = 0;

    open_file(tmp_path, "r", &tmp_fptr);

    while (read_ptr != NULL && i < keys->num_keys)
    {
        read_ptr = fgets(cur_str, max_str_len, tmp_fptr);

        if (read_ptr != NULL && strcmp(cur_str, "\n") != 0)
        {
            // replace newline with null byte
            cur_str[strlen(cur_str) - 1] = '\0';

            cur_key = keys->keyarr[i];
            free(map[cur_key]->dest);

            map[cur_key]->dest = malloc(max_str_len * sizeof(map[cur_key]->dest));
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

/**
 * @brief iterate over hashmap of name pairs using list of keys and
 *      change sources (old names) to destinations (new names)
 *
 * @param map: hashmap of old and new names
 * @param keys: struct containing list of keys to nodes in hashmap
 */
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

/**
 * @brief rename an item in file system
 *
 * @param src: old name
 * @param dest: new name
 */
void rename_path_pair(const char *src, const char *dest)
{
    int rename_result = rename(src, dest);

    if (rename_result == -1)
        fprintf(stderr, "mmv: failed to rename \"%s\" to \"%s\"\n", src, dest);
}

/**
 * @brief delete an item at the given path in the file system
 *
 * @param path: path to item to delete
 */
void rm_path(char *path)
{
    int rm_success = remove(path);

    if (rm_success == -1)
        fprintf(stderr, "mmv: failed to delete \"%s\"\n", path);
}

/**
 * @brief iterate over map of nodes containing item names to rename
 *      and write said names to the given file pointer
 *
 * @param fptr: pointer to open file to write to
 * @param map: map of string pair nodes, contains old names to write
 *      to fptr
 * @param keys: struct containing list of keys to node locations in
 *      hashmap
 */
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

/**
 * @brief open temp file at path, write source strings (old names)
 *      to it, and close said temp file
 *
 * @param path: path to open and write to
 * @param map: map of source strings to write
 * @param keys: struct containing list of keys to node locations in
 *      hashmap
 */
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
