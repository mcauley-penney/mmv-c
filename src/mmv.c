#include "./mmv.h"

struct Opts *make_opts(void)
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

struct Set *set_init(
    bool resolve_paths, const int arg_count, char *args[], bool track_dupes
)
{
	if (arg_count == 0)
	{
		fputs("mmv: missing file operand(s)", stderr);
		return NULL;
	}

	unsigned int i;
	const unsigned int u_arg_count       = (unsigned int)arg_count;
	const unsigned long int map_capacity = (6 * (u_arg_count - 1)) + 1;

	struct Set *set = set_alloc(map_capacity);
	if (set == NULL)
	{
		perror("mmv: failed to allocate memory to initialize string set\n");
		return NULL;
	}

	set->map_capacity = map_capacity;

	char *cur_str;

	for (i = 0; i < u_arg_count; i++)
	{
		cur_str = args[i];
		if (resolve_paths) cur_str = realpath(cur_str, NULL);

		if (set_insert(cur_str, set, track_dupes) == -1)
		{
			set_destroy(set);
			fprintf(
			    stderr, "mmv: failed to insert \'%s\': %s\n", cur_str,
			    strerror(errno)
			);
			return NULL;
		}
	}

	return set;
}

struct Set *set_alloc(const unsigned long int map_capacity)
{
	unsigned int i;

	struct Set *set = malloc(sizeof(struct Set));
	if (set == NULL) return NULL;

	set->map = malloc(sizeof(char *) * map_capacity);
	if (set->map == NULL)
	{
		free(set);
		return NULL;
	}

	set->num_keys = 0;

	for (i = 0; i < map_capacity; i++)
		set->map[i] = NULL;

	return set;
}

int set_insert(char *cur_str, struct Set *set, bool track_dupes)
{
	long unsigned int hash = hash_str(cur_str, set->map_capacity);
	int is_dupe            = is_duplicate_element(cur_str, set, &hash);

	if (is_dupe == 0)
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

long unsigned int hash_str(char *str, const unsigned long int map_capacity)
{
	unsigned char *s = (unsigned char *)str;
	Fnv32_t hval     = ((Fnv32_t)0x811c9dc5);

	while (*s)
	{
		hval ^= (Fnv32_t)*s++;
		hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) +
		        (hval << 24);
	}

	return (long unsigned int)(hval % map_capacity);
}

char *cpy_str_to_arr(char **arr_dest, const char *src_str)
{
	*arr_dest = malloc((strlen(src_str) + 1) * sizeof(char));
	if (arr_dest == NULL)
	{
		perror("mmv: failed to allocate memory for new map str\n");
		return NULL;
	}

	return strcpy(*arr_dest, src_str);
}

int write_strarr_to_tmpfile(struct Set *map, char tmp_path_template[])
{
	size_t i;

	FILE *tmp_fptr = open_tmpfile_fptr(tmp_path_template);
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

	return 0;
}

FILE *__attribute__((malloc)) open_tmpfile_fptr(char *tmp_path)
{
	int tmp_fd = mkstemp(tmp_path);
	if (tmp_fd == -1) return NULL;

	return fdopen(tmp_fd, "w");
}

int edit_tmpfile(const char *path)
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

	return 0;
}

struct Set *init_dest_set(unsigned int num_keys, char path[])
{
	// size of destination array only needs to be, at
	// maximum, the number of keys in the source set
	char **dest_arr = malloc(sizeof(char *) * num_keys);
	if (dest_arr == NULL) return NULL;

	int dest_size = 0;

	if (read_tmpfile_strs(dest_arr, &dest_size, num_keys, path) != 0)
	{
		free_strarr(dest_arr, dest_size);
		return NULL;
	}

	struct Set *set = set_init(false, dest_size, dest_arr, true);

	free_strarr(dest_arr, dest_size);

	return set;
}

int read_tmpfile_strs(
    char **dest_arr, int *dest_size, unsigned int num_keys, char path[]
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

	while (read_ptr != NULL && i < num_keys)
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

	return 0;
}

void free_strarr(char **arr, int arr_size)
{
	for (int i = 0; i < arr_size; i++)
		free(arr[i]);

	free(arr);
}

int rename_paths(struct Set *src_set, struct Set *dest_set, struct Opts *opts)
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
			rename_path(src_str, dest_set->map[dest_key], opts);

		else
			printf(
			    "mmv: duplicate dest found for src '%s'. No mv conducted.\n",
			    src_str
			);
	}

	return 0;
}

void rename_path(const char *src, const char *dest, struct Opts *opts)
{
	if (strcmp(src, dest) == 0) return;

	if (rename(src, dest) == -1)
		fprintf(
		    stderr, "mmv: \'%s\' to \'%s\': %s\n", src, dest, strerror(errno)
		);

	else if (opts->verbose)
		printf("  '%s' -> '%s'\n", src, dest);
}

void rm_path(char *path)
{
	if (remove(path) == -1)
		fprintf(
		    stderr, "mmv: failed to delete \'%s\': %s\n", path, strerror(errno)
		);
}

void set_destroy(struct Set *map)
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

int rm_cycles(struct Set *src_set, struct Set *dest_set, struct Opts *opts)
{
	size_t i;
	int cur_src_key, cur_dest_key, is_dupe;
	unsigned long int u_key;
	char *cur_src_str, *cur_dest_str;

	for (i = 0; i < dest_set->num_keys; i++)
	{
		cur_src_key  = src_set->keys[i];
		cur_dest_key = dest_set->keys[i];
		cur_src_str  = src_set->map[cur_src_key];
		cur_dest_str = dest_set->map[cur_dest_key];

		if (cur_dest_key != -1 && strcmp(cur_src_str, cur_dest_str) != 0)
		{
			u_key   = (unsigned int)cur_dest_key;
			is_dupe = is_duplicate_element(cur_dest_str, src_set, &u_key);
			char tmp_path[] = "mmv_cycle_XXXXXX";

			if (is_dupe == 0)
			{
				// create temporary name using the current name
				int tmp_fd = mkstemp(tmp_path);
				if (tmp_fd == -1) return -1;

				// rename to temporary name
				rename_path(src_set->map[u_key], tmp_path, opts);

				// update str in src map to temp_str
				free(src_set->map[u_key]);
				cpy_str_to_arr(&src_set->map[u_key], tmp_path);
			}
		}
	}

	return 0;
}

int is_duplicate_element(
    char *cur_str, struct Set *set, long unsigned int *hash
)
{
	int dupe_found = -1;

	while (set->map[*hash] != NULL && dupe_found != 0)
	{
		// strcmp returns 0 if strings are identical
		dupe_found = strcmp(set->map[*hash], cur_str);

		if (dupe_found != 0)
			*hash = (*hash + 1 < set->map_capacity) ? *hash + 1 : 0;
	}

	return dupe_found;
}
