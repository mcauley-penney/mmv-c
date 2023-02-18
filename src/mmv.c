#include "mmv.h"

struct Opts *make_opts(void)
{
	struct Opts *opts = malloc(sizeof(struct Opts));
	if (opts == NULL)
	{
		perror("mmv: failed to allocate memory for user flags\n");
		return NULL;
	}

	opts->verbose = false;

	return opts;
}

void usage(void)
{
	printf("Usage: %s [OPTION] SOURCES\n\n", PROG_NAME);
	puts("Rename or move SOURCE(s) by editing them in $EDITOR.");
	printf("For full documentation, see man %s\n", PROG_NAME);
}

void try_help(void)
{
	puts("Try 'mmv -h'for more information");
}

struct Set *make_str_set(const int arg_count, char *args[], bool track_dupes)
{
	if (arg_count == 0)
	{
		fprintf(stderr, "mmv: missing file operand\n");
		return NULL;
	}

	unsigned int i;
	const unsigned int u_arg_count = (unsigned int)arg_count;

	// (6 * (val - 1)) + 1 is a formula that creates a
	// prime number using some value as a base. We are
	// using this to create a prime hash map size to
	// attempt to mitigate collisions. While the FNV
	// hash is known to be of high quality and this is
	// supposed to not be necessary, this is not heavy
	// to implement and adds a layer of protection.
	const unsigned int map_capacity = (6 * (u_arg_count - 1)) + 1;

	struct Set *set = alloc_str_set(u_arg_count, map_capacity);
	if (set == NULL)
	{
		perror("mmv: failed to allocate memory to initialize hashmap");
		return NULL;
	}

	for (i = 0; i < u_arg_count; i++)
		if (str_set_insert(args[i], map_capacity, set, track_dupes) == -1)
		{
			free_str_set(set);
			return NULL;
		}

	return set;
}

struct Set *alloc_str_set(
    const unsigned int num_names, const unsigned int map_size
)
{
	unsigned int i;

	struct Set *set = malloc(sizeof(struct Set) + num_names * sizeof(int));
	if (set == NULL) return NULL;

	set->map = malloc(sizeof(char *) * map_size);
	if (set->map == NULL)
	{
		free(set);
		return NULL;
	}

	set->num_keys = 0;

	for (i = 0; i < map_size; i++)
		set->map[i] = NULL;

	return set;
}

int str_set_insert(
    char *cur_str, const unsigned int map_space, struct Set *set,
    bool track_dupes
)
{
	int dupe_found    = -1;
	unsigned int hash = hash_str(cur_str, map_space);

	while (set->map[hash] != NULL && dupe_found != 0)
	{
		// strcmp returns 0 if strings are identical
		dupe_found = strcmp(set->map[hash], cur_str);

		hash = (hash + 1 < map_space) ? hash + 1 : 0;
	}
	if (dupe_found == 0)
	{
		if (track_dupes)
		{
			set->keys[set->num_keys] = -1;
			set->num_keys++;
		}

		return 0;
	}

	if (cpy_str_to_arr(&set->map[hash], cur_str) == NULL) return -1;

	set->keys[set->num_keys] = (int)hash;
	set->num_keys++;

	return 1;
}

unsigned int hash_str(char *str, const unsigned int set_capacity)
{
	unsigned char *s = (unsigned char *)str;
	Fnv32_t hval     = ((Fnv32_t)0x811c9dc5);

	while (*s)
	{
		hval ^= (Fnv32_t)*s++;
		hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) +
		        (hval << 24);
	}

	return hval % set_capacity;
}

char *cpy_str_to_arr(char **arr_dest, const char *src_str)
{
	*arr_dest = malloc((strlen(src_str) + 1) * sizeof(char));
	if (arr_dest == NULL)
	{
		perror("mmv: failed to allocate memory for new map node source str\n");
		return NULL;
	}

	return strcpy(*arr_dest, src_str);
}

int write_strarr_to_tmpfile(struct Set *map, char tmp_path_template[])
{
	size_t i;

	FILE *tmp_fptr = open_tmp_path_fptr(tmp_path_template);
	if (tmp_fptr == NULL)
	{
		fprintf(
		    stderr, "mmv: failed to open \"%s\": %s\n", tmp_path_template,
		    strerror(errno)
		);
		return errno;
	}

	for (i = 0; i < map->num_keys; i++)
		fprintf(tmp_fptr, "%s\n", map->map[map->keys[i]]);

	fclose(tmp_fptr);

	return EXIT_SUCCESS;
}

FILE *__attribute__((malloc)) open_tmp_path_fptr(char *tmp_path)
{
	int tmp_fd = mkstemp(tmp_path);
	if (tmp_fd == -1) return NULL;

	return fdopen(tmp_fd, "w");
}

int open_file_in_editor(const char *path)
{
	int ret;
	char *editor_name = getenv("EDITOR");
	if (editor_name == NULL) editor_name = "nano";

	// provide space for "$EDITOR <path>\0", e.g. "nano test.txt\0"
	const size_t cmd_len = strlen(editor_name) + strlen(path) + 2;

	char *edit_cmd = malloc(sizeof(edit_cmd) * cmd_len);
	if (edit_cmd == NULL)
	{
		perror("mmv: failed to allocate memory for $EDITOR command string");
		return errno;
	}

	ret = snprintf(edit_cmd, cmd_len, "%s %s", editor_name, path);
	if (ret < 0 || ret > (int)cmd_len)
	{
		perror("mmv: couldn't create $EDITOR command string");
		free(edit_cmd);
		return errno;
	}

#if DEBUG == 0
	if (system(edit_cmd) != 0)
	{
		fprintf(
		    stderr, "mmv: \'%s\' returned non-zero exit status: %d\n",
		    editor_name, ret
		);
		free(edit_cmd);
		return errno;
	}
#endif

	free(edit_cmd);

	return EXIT_SUCCESS;
}

struct Set *make_dest_str_set(struct Set *src_set, char path[])
{
	// size of destination array only needs to be, at
	// maximum, the number of keys in the source set
	char **dest_arr = malloc(sizeof(char *) * src_set->num_keys);
	if (dest_arr == NULL) return NULL;

	int dest_size = 0;

	if (read_tmp_file_strs(dest_arr, &dest_size, src_set, path) != 0)
	{
		free_str_arr(dest_arr, dest_size);
		return NULL;
	}

	struct Set *set = make_str_set(dest_size, dest_arr, true);

	free_str_arr(dest_arr, dest_size);

	return set;
}

int read_tmp_file_strs(
    char **dest_arr, int *dest_size, struct Set *set, char path[]
)
{
	char cur_str[PATH_MAX], *read_ptr = "";
	size_t i = 0;

	FILE *tmp_fptr = fopen(path, "r");
	if (tmp_fptr == NULL)
	{
		fprintf(
		    stderr, "mmv: failed to open \"%s\" in \"r\" mode: %s\n", path,
		    strerror(errno)
		);
		return errno;
	}

	while (read_ptr != NULL && i < set->num_keys)
	{
		read_ptr = fgets(cur_str, PATH_MAX, tmp_fptr);

		if (read_ptr != NULL && strcmp(cur_str, "\n") != 0)
		{
			cur_str[strlen(cur_str) - 1] = '\0';

			cpy_str_to_arr(&dest_arr[(*dest_size)], cur_str);
			(*dest_size)++;

			i++;
		}
	}

	fclose(tmp_fptr);

	return EXIT_SUCCESS;
}

void free_str_arr(char **arr, int arr_size)
{
	for (int i = 0; i < arr_size; i++)
		free(arr[i]);

	free(arr);
}

int rename_filesystem_items(
    struct Opts *options, struct Set *src_set, struct Set *dest_set
)
{
	size_t i;
	int src_key, dest_key;
	char *src_str;

	for (i = 0; i < src_set->num_keys; i++)
	{
		dest_key = dest_set->keys[i];
		src_key  = src_set->keys[i];
		src_str  = src_set->map[src_key];

		if (dest_key != -1)
			rename_path(options, src_str, dest_set->map[dest_key]);

		else
			printf(
			    "mmv: duplicate dest found for src '%s'. No rename "
			    "conducted.\n",
			    src_str
			);
	}

	return EXIT_SUCCESS;
}

void rename_path(struct Opts *options, const char *src, const char *dest)
{
	if (rename(src, dest) == -1)
		fprintf(
		    stderr, "mmv: \'%s\' to \'%s\': %s\n", src, dest, strerror(errno)
		);

	else if (options->verbose)
		printf("renamed '%s' -> '%s'\n", src, dest);
}

void rm_path(char *path)
{
	if (remove(path) == -1)
		fprintf(
		    stderr, "mmv: failed to delete \'%s\': %s\n", path, strerror(errno)
		);
}

void free_str_set(struct Set *map)
{
	size_t i;
	int key;

	for (i = 0; i < map->num_keys; i++)
	{
		key = map->keys[i];

		if (key != -1) free(map->map[key]);
	}

	free(map->map);
	free(map);
}
