#include "src/mmv.h"

int main(int argc, char *argv[])
{
	int flag_arg;
	char short_opts[]				 = "hvV";
	int option_index				 = 0;
	static struct option long_opts[] = {
		{"help", no_argument, NULL, 'h'},
		{"verbose", no_argument, NULL, 'v'},
		{"version", no_argument, NULL, 'V'},
	};
	struct Opts *options = make_opts();

	while ((flag_arg = getopt_long(
				argc, argv, short_opts, long_opts, &option_index)) != -1)
	{
		switch (flag_arg)
		{
		case 'h':
			free(options);
			usage();
			return EXIT_SUCCESS;

		case 'v':
			options->verbose = true;
			break;

		case 'V':
			free(options);
			printf("mmv version %s\n", PROG_VERSION);
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

struct Opts *make_opts()
{
	struct Opts *opts = malloc(sizeof(struct Opts));
	if (opts == NULL)
		return NULL;

	opts->verbose = false;

	return opts;
}

void try_help()
{
	puts("Try 'mmv [-h][--help]'for more information");
}

void usage()
{
	printf("Usage: %s [OPTION] SOURCES\n\n", PROG_NAME);
	puts("Rename or move SOURCE(s) by editing them in $EDITOR.");
	printf("For full documentation, see man %s\n", PROG_NAME);
}
