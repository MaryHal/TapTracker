#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tracker.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("This program expects shmupmametgm as its first argument. Each subsequent argument is passed to shmupmametgm.");
        return 1;
    }

    const char* sharedMemKey = "tgm2p_data";
    int fd = shm_open(sharedMemKey, O_RDWR | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
    if (fd < 0)
    {
        perror("Could not create shared memory object");
    }

    size_t vSize = sizeof(int) * 4;

    // Stretch our new file to the suggested size.
    if (lseek(fd, vSize - 1, SEEK_SET) == -1)
    {
	perror("Could not stretch file via lseek");
        goto file_error;
    }

    // In order to change the size of the file, we need to actually write some
    // data. In this case, we'll be writing an empty string ('\0').
    if (write(fd, "", 1) != 1)
    {
	perror("Could not write the final byte in file");
        goto file_error;
    }

    int* addr = mmap(NULL, vSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("Parent: Could not map memory");
        goto map_error;
    }

    // Lock the mapped region into memory
    if(mlock(addr, vSize) != 0)
    {
        perror("mlock failure");
        goto lock_error;
    }

    int subprocessPid = fork();
    if (subprocessPid < 0) // Uh oh
    {
        perror("Could not fork process");
        exit(1);
    }
    else if (subprocessPid == 0) // Child
    {
        // Execute MAME. argv is guaranteed to be NULL-terminated, very
        // convenient.
        execv(argv[1], argv + 1);
    }
    else // Parent
    {
        runTracker(subprocessPid, addr);

        // Wait for child to finish
        printf("Parent process completed. Waiting for child to exit...\n");
        int childStatus = 0;
        wait(&childStatus);
    }

    if (munlock(addr, vSize) != 0)
        perror("Error unlocking memory");

lock_error:
    if (munmap(addr, vSize) != 0)      /* Unmap the page */
        perror("Error unmapping memory pointer");

map_error:
    if (close(fd) != 0)                /* Close file */
        perror("Error closing file");

file_error:
    if (shm_unlink(sharedMemKey) != 0) /* Unlink shared-memory object */
        perror("Error removing memory object");

    return 0;
}
