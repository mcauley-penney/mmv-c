#include "src/mmv.h"

int main(int argc, char *argv[])
{
    struct Set *mmv_set = make_str_set(argc, argv);
    if (mmv_set == NULL)
        exit(EXIT_FAILURE);

    char tmp_path[] = "/tmp/mmv_XXXXXX";

    umask(077);

    if (write_strarr_to_tmpfile(mmv_set, tmp_path) != 0)
        goto rm_path_out;

    if (open_file_in_editor(tmp_path) != 0)
        goto rm_path_out;

    if (rename_filesystem_items(mmv_set, tmp_path) != 0)
        goto rm_path_out;

    rm_path(tmp_path);
    free_str_set(mmv_set);

    return EXIT_SUCCESS;

rm_path_out:
    rm_path(tmp_path);
    free_str_set(mmv_set);
    return EXIT_FAILURE;
}
