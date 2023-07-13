#include "src/mmv.h"

int main(int argc, char *argv[])
{
	int cur_flag;
	char short_opts[] = "rhvV";

	struct Opts *options = make_opts();
	if (options == NULL) return EXIT_FAILURE;

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
				try_help();
				return EXIT_FAILURE;
		}
	}

	argv += optind;
	argc -= optind;

	if (argc > MAX_OPS)
	{
		fprintf(stderr, "mmv: too many operands, use up to %u\n", MAX_OPS);
		goto free_opts_out;
	}

	struct Set *src_set = set_init(options->resolve_paths, argc, argv, false);
	if (src_set == NULL) goto free_opts_out;

	char tmp_path[] = "/tmp/mmv_XXXXXX";

	umask(077);

	if (write_strarr_to_tmpfile(src_set, tmp_path) != 0) goto free_src_out;

	if (edit_tmpfile(tmp_path) != 0) goto rm_path_out;

	struct Set *dest_set = init_dest_set(src_set->num_keys, tmp_path);
	if (dest_set == NULL) goto rm_path_out;

	if (rm_cycles(src_set, dest_set, options) != 0) goto free_dest_out;
	rename_paths(src_set, dest_set, options);

	set_destroy(dest_set);
	rm_path(tmp_path);
	set_destroy(src_set);
	free(options);


	return EXIT_SUCCESS;


free_dest_out:
	set_destroy(dest_set);

rm_path_out:
	rm_path(tmp_path);

free_src_out:
	set_destroy(src_set);

free_opts_out:
	free(options);
	return EXIT_FAILURE;
}
