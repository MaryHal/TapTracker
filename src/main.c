#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>

#include "tap_state.h"
#include "tracker.h"

int main(int argc, char *argv[])
{
    (void) argc, (void)argv;

    const char* sharedMemKey = "tgm2p_data";
    int fd = shm_open(sharedMemKey, O_RDONLY, S_IRWXO | S_IRWXG | S_IRWXU);
    if (fd < 0)
    {
        perror("Could not open shared memory object");
        return 1;
    }

    const size_t vSize = sizeof(struct tap_state);

    struct tap_state* addr = (struct tap_state*)mmap(NULL, vSize, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("Could not map memory");
        return 1;
    }

    struct tracker_settings_t settings =
    {
        .jmap =
        {
            // Joystick  A  B  C
            .buttons = { 1, 2, 3 },
            .axisHori = 6,
            .axisVert = 7
        },
        .enableJoystick = true
    };
    runTracker(addr, settings);

    if (munmap(addr, vSize) != 0)
        perror("Error unmapping memory pointer");

    if (close(fd) != 0)
        perror("Error closing file");

    return 0;
}
