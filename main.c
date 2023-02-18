#include "src/mmv.h"

int main(int argc, char *argv[])
{
	int cur_flag;
	char short_opts[] = "hvV";

	struct Opts *options = make_opts();
	if (options == NULL) return EXIT_FAILURE;

	while ((cur_flag = getopt(argc, argv, short_opts)) != -1)
	{
		switch (cur_flag)
		{
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

	struct Set *src_set = make_str_set(argc, argv, false);
	if (src_set == NULL) goto free_opts_out;

	char tmp_path[] = "/tmp/mmv_XXXXXX";

	umask(077);

	if (write_strarr_to_tmpfile(src_set, tmp_path) != 0) goto free_src_out;

	if (open_file_in_editor(tmp_path) != 0) goto rm_path_out;

	struct Set *dest_set = make_dest_str_set(src_set, tmp_path);
	if (dest_set == NULL) goto rm_path_out;

	rename_filesystem_items(options, src_set, dest_set);

	free_str_set(dest_set);
	rm_path(tmp_path);
	free_str_set(src_set);
	free(options);


	return EXIT_SUCCESS;


rm_path_out:
	rm_path(tmp_path);

free_src_out:
	free_str_set(src_set);

free_opts_out:
	free(options);
	return EXIT_FAILURE;
}
