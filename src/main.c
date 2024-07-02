#include <getopt.h>

#include "mmv.h"

/**
 * @brief Create struct of possible user flags
 * @return Opt struct
 */
static struct Opts *make_opts(void)
{
    struct Opts *opts = malloc(sizeof(struct Opts));
    if (opts == NULL)
    {
        perror("mmv: failed to allocate memory for user flags\n");
        return NULL;
    }

    opts->resolve_paths = false;
    opts->verbose       = false;

    return opts;
}

/**
 * @brief Print program usage information
 */
static void usage(void)
{
    printf("Usage: %s [OPTION] SOURCES\n\n", PROG_NAME);
    puts("Rename or move SOURCE(s) by editing them in $EDITOR.");
    printf("For full documentation, see man %s\n", PROG_NAME);
}

int main(int argc, char *argv[])
{
    int cur_flag;
    char short_opts[] = "rhvV";

    struct Opts *options = make_opts();
    if (options == NULL)
        return EXIT_FAILURE;

    while ((cur_flag = getopt(argc, argv, short_opts)) != -1)
    {
        switch (cur_flag)
        {
            case 'r': options->resolve_paths = true; break;
            case 'v': options->verbose = true; break;

            case 'h':
                free(options);
                usage();
                return EXIT_SUCCESS;

            case 'V':
                free(options);
                puts(PROG_VERSION);
                return EXIT_SUCCESS;

            default:
                free(options);
                puts("Try 'mmv -h'for more information");
                return EXIT_FAILURE;
        }
    }

    argv += optind;
    argc -= optind;

    struct Set *src_set = init_src_set(argc, argv, options);
    if (src_set == NULL)
        goto free_opts_out;

    char tmpfile[] = "/tmp/mmv_editbuf_XXXXXX";

    if (write_strarr_to_tmpfile(src_set, tmpfile) != 0)
        goto free_src_out;

    if (edit_tmpfile(tmpfile) != 0)
        goto rm_path_out;

    struct Set *dest_set = init_dest_set(src_set->num_keys, tmpfile);
    if (dest_set == NULL)
        goto rm_path_out;

    if (rm_unedited_pairs(src_set, dest_set, options) != 0)
        goto free_dest_out;

    if (argc > 1 && rm_cycles(src_set, dest_set, options) != 0)
        goto free_dest_out;

    rename_paths(src_set, dest_set, options);

    set_destroy(dest_set);
    remove(tmpfile);
    set_destroy(src_set);
    free(options);

    return EXIT_SUCCESS;


free_dest_out:
    set_destroy(dest_set);

rm_path_out:
    remove(tmpfile);

free_src_out:
    set_destroy(src_set);

free_opts_out:
    free(options);
    return EXIT_FAILURE;
}
