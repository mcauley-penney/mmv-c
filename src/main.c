#include "mmv.h"

int main(int argc, char *argv[])
{
    struct Map *rename_map = make_str_hashmap(argc, argv);
    if (rename_map == NULL)
        exit(EXIT_FAILURE);

    char tmp_path[] = "/tmp/mmv_XXXXXX";

    umask(077);

    if (write_strarr_to_tmpfile(rename_map, tmp_path) != 0)
        goto rm_path_out;

    if (open_file_in_editor(tmp_path) != 0)
        goto rm_path_out;

    if (rename_filesystem_items(rename_map, tmp_path) != 0)
        goto rm_path_out;

    rm_path(tmp_path);
    free_hashmap(rename_map);

    return EXIT_SUCCESS;

rm_path_out:
    rm_path(tmp_path);
    free_hashmap(rename_map);
    return EXIT_FAILURE;
}
