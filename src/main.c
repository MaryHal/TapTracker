#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>

#include "tracker.h"

int main(int argc, char *argv[])
{
    (void) argc, (void)argv;

    const char* sharedMemKey = "tgm2p_data";
    int fd = shm_open(sharedMemKey, O_RDWR, S_IRWXO | S_IRWXG | S_IRWXU);
    if (fd < 0)
    {
        perror("Could not create shared memory object");
        return 1;
    }

    size_t vSize = sizeof(int) * 4;

    int* addr = mmap(NULL, vSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("Parent: Could not map memory");
    }

    runTracker(addr);

    if (munmap(addr, vSize) != 0)      /* Unmap the page */
        perror("Error unmapping memory pointer");

    if (close(fd) != 0)                /* Close file */
        perror("Error closing file");

    return 0;
}
