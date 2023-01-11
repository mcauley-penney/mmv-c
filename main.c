#include "src/mmv.h"

int main(int argc, char *argv[])
{
	int cur_flag;
	char short_opts[]	 = "hvV";

	struct Opts *options = make_opts();
	if (options == NULL)
		return EXIT_FAILURE;

	while ((cur_flag = getopt(argc, argv, short_opts)) != -1)
	{
		switch (cur_flag)
		{
		case 'v':
			options->verbose = true;
			break;

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

	struct Set *mmv_set = make_str_set(argc, argv);
	if (mmv_set == NULL)
	{
		free(options);
		return EXIT_FAILURE;
	}

	char tmp_path[] = "/tmp/mmv_XXXXXX";

	umask(077);

	if (write_strarr_to_tmpfile(mmv_set, tmp_path) != 0)
		goto rm_path_out;

	if (open_file_in_editor(tmp_path) != 0)
		goto rm_path_out;

	if (rename_filesystem_items(options, mmv_set, tmp_path) != 0)
		goto rm_path_out;

	rm_path(tmp_path);
	free(options);
	free_str_set(mmv_set);

	return EXIT_SUCCESS;

rm_path_out:
	rm_path(tmp_path);
	free(options);
	free_str_set(mmv_set);
	return EXIT_FAILURE;
}
