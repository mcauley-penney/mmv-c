#include "./set.h"

/**
 * @brief Allocate memory for members of a Set struct
 *
 * @param num_args
 * @param map_size
 */
static struct Set *set_alloc(const unsigned long int map_capacity)
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

/**
 * @brief hashes a string with the Fowler–Noll–Vo 1a 32bit hash fn
 *
 * @param str: string to hash
 * @param map_size: size of map; used to modulo the hash to fit into array
 *
 * @return hash for input string % map_size
 */
static long unsigned int hash_str(
    char *str, const unsigned long int map_capacity
)
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

/**
 * @brief Insert a string into the given Set struct
 *
 * @param cur_str
 * @param map_size
 * @param map
 * @return
 */
static int set_insert(char *cur_str, struct Set *set, bool track_dupes)
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
		perror("mmv: failed to allocate memory to initialize string set");
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

int *set_begin(struct Set *map)
{
	return &map->keys[0];
}

int *set_next(int *iter)
{
	return ++iter;
}

int *set_end(struct Set *map)
{
	return &map->keys[map->num_keys];
}

char *get_set_str_at_iter(const struct Set *map, const int *iter)
{
	return map->map[*iter];
}

int is_invalid_key(const int *iter)
{
	return *iter == -1;
}
