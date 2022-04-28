/**
 * TODO:
 * 1. replace unsafe functions with safe ones
 * 2. produce documentation
 * 3. determine what datatype to use for numerical values here
 * 4. make param names uniform, e.g. arg_count vs arr_len, etc.
 * 5. enforce C naming conventions
 * 7. group functions in main into parent functions after done
 * 8. alphabetize fns and remove unused defs
 */

#include "mmv.h"
#include <time.h>

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
    const int tmp_path_len = strlen(tmp_path);
    FILE *tmp_fptr;
    int hash, i, insert_key, keyarr[argc], keyarr_len = 0, map_size = (6 * argc) + 1;
    pair *map[map_size];

    for (i = 0; i < map_size; i++)
        map[i] = NULL;

    for (i = 1; i < argc; i++)
    {
        hash = fnv_32a_str(argv[i]) % map_size;

        insert_key = hashmap_insert(map, argv[i], hash);

        if (insert_key == 1)
        {
            keyarr[keyarr_len] = hash;
            keyarr_len++;
        }
    }

    // ------------------------------------------------------------------------
#ifdef TESTING
    print_map(map, keyarr, keyarr_len);
#endif
    // ------------------------------------------------------------------------

    tmp_fptr = get_tmp_path_fptr(tmp_path);
    write_map_to_fptr(tmp_fptr, map, keyarr, keyarr_len);
    // there is no corresponding explicit fopen() call for this fclose()
    // because mkstemp in mk_uniq_path() opens temp file for us
    fclose(tmp_fptr);

    open_file_in_buf(tmp_path, tmp_path_len);

    open_file(tmp_path, "r", &tmp_fptr);
    read_lines_from_fptr(tmp_fptr, map, keyarr, keyarr_len);
    fclose(tmp_fptr);

    rename_files(map, keyarr, keyarr_len);

    rm_path(tmp_path);
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

// ----------------------------------------------------------------------------

Fnv32_t fnv_32a_str(char *str)
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

    return hval;
}

void free_map(pair *map[], const int keyarr[], const int keyarr_len)
{
    int i;

    for (i = 0; i < keyarr_len; i++)
    {
        pair *wkg_node = map[keyarr[i]];

        free_pair_ll(wkg_node);
    }
}

void free_pair_ll(pair *node)
{
    if (node != NULL)
    {
        free_pair_ll(node->next);

        free(node->dest);
        free(node->src);
        free(node);

        node = NULL;
    }
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

int hashmap_insert(pair *map[], char *str, int hash)
{
    int is_root = 1;

    if (map[hash] == NULL)
        map[hash] = init_pair_node(str);

    else
    {
        is_root = 0;
        pair *parent = map[hash], *wkg_node = map[hash];
        while (wkg_node != NULL)
        {
            if (strcmp(wkg_node->src, str) == 0)
                return -1;

            parent = wkg_node;
            wkg_node = wkg_node->next;
        }

        parent->next = init_pair_node(str);
    }

    return is_root;
}

pair *init_pair_node(char *src_str)
{
    pair *new_node = malloc(sizeof(pair));
    new_node->dest = NULL;
    new_node->next = NULL;
    new_node->src = malloc((strlen(src_str) + 1) * sizeof(char));
    strcpy(new_node->src, src_str);

    return new_node;
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

void print_map(pair *map[], int keyarr[], int keyarr_len)
{
    int i;

    printf("\n");

    for (i = 0; i < keyarr_len; i++)
    {
        int key = keyarr[i], pos = 0;
        pair *wkg_node = map[key];

        while (wkg_node != NULL)
        {
            printf("node_src: %s, node pos: %d\n", wkg_node->src, pos);
            printf("key pos: %d, key: %d\n", i, key);
            wkg_node = wkg_node->next;
            pos++;
        }
    }
}

void read_lines_from_fptr(FILE *fptr, pair *map[], const int keyarr[], const int keyarr_len)
{
    // TODO: must do more to protect against buffer overflow
    const int max_str_len = 500;

    // TODO: does this need a malloc? does the read_ptr?
    char *cur_str = malloc(max_str_len * sizeof(cur_str)), *read_ptr = "";
    int cur_key, i = 0;

    while (i < keyarr_len && read_ptr != NULL)
    {
        read_ptr = fgets(cur_str, max_str_len, fptr);

        if (read_ptr != NULL && strcmp(cur_str, "\n") != 0)
        {
            cur_key = keyarr[i];
            map[cur_key]->dest = malloc(max_str_len * sizeof(char));
            rm_str_nl(cur_str);
            strcpy(map[cur_key]->dest, cur_str);
            i++;
        }
    }

    free(cur_str);
}

void rename_files(pair *map[], const int keyarr[], const int keyarr_len)
{
    /**
     * TODO: add protections
     *  - disallow duplicate renames
     *  - allow swapping and cycling names
     */
    int i, rename_result;

    for (i = 0; i < keyarr_len; i++)
    {
        pair *wkg_node = map[keyarr[i]];

        while (wkg_node != NULL)
        {
            rename_result = rename(wkg_node->src, wkg_node->dest);

            if (rename_result == -1)
                fprintf(stderr, "ERROR: Could not rename \"%s\" to \"%s\"\n", wkg_node->src, wkg_node->dest);

            wkg_node = wkg_node->next;
        }
    }
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

void write_map_to_fptr(FILE *fptr, pair *map[], int keys[], const int num_keys)
{
    int i;
    pair *wkg_node;

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
