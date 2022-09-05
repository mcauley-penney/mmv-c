#include "test_mmv.h"
#include <sys/stat.h>

int main(void)
{
    printf("--BEGINNING UNIT TESTING--\n");

    printf("Testing set creation...\n");
    verify_correct_set_creation();
    printf("Done!\n");

    printf("Testing file output capabilities...\n");
    verify_correct_write_to_file();
    printf("Done!\n");

    return EXIT_SUCCESS;
}
