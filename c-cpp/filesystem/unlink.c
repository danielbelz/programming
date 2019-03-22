#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Filesystem function to delete the list of files
 *
 * @param   argc    Number of arguments in argv
 * @param   argv    Name of the files to be unlinked (deleted)
 * @return  int     Return value of the unlink function call
 */


int main(int argc, char *argv[])
{
    int delete_status = -1;
    int i;

    // argv[0] is the executable name, skip it.
    for (i=1; argc > 1 && i < argc; i++) {
        printf("Deleting file %s (%d of %d)...", argv[i], i, argc-1);
        delete_status = unlink(argv[i]);
        printf("status return is %d\n", delete_status);
    }

    return delete_status;
}