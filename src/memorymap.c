#include "memorymap.h"
#include "tap_state.h"

#include <zf_log.h>

static size_t vSize = sizeof(struct tap_state);

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

struct tap_state* getMappingPtr()
{
    const char* sharedMemKey = "taptracker_data";
    int fd = shm_open(sharedMemKey, O_RDONLY, S_IRWXO | S_IRWXG | S_IRWXU);
    if (fd < 0)
    {
        ZF_LOGF("Could not open shared memory object");
        return 1;
    }

    struct tap_state* addr = (struct tap_state*)mmap(NULL, vSize, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        ZF_LOGF("Could not map memory");
        return 1;
    }

    return addr;
}

void freeMappingPtr(struct tap_state* m)
{
    if (munmap(m, vSize) != 0)
        ZF_LOGF("Error unmapping memory pointer");

    if (close(fd) != 0)
        ZF_LOGF("Error closing file");
}

#else

#include <windows.h>
#include <stdio.h>

static const char* sharedMemFilename = "taptracker_data";
static HANDLE fileHandle;
static HANDLE mapping;
static LPVOID view;

struct tap_state* getMappingPtr()
{
    char filepath[256];
    snprintf(filepath, 256, "%s\\%s", getenv("APPDATA"), sharedMemFilename);

    fileHandle = CreateFile(
        filepath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        ZF_LOGF("CreateFile has failed");
        exit(1);
    }

    mapping = CreateFileMapping(
        fileHandle,
        NULL,
        PAGE_READWRITE,
        0,
        vSize,
        NULL);

    if (mapping == NULL)
    {
        ZF_LOGF("CreateFileMapping has failed.");
    }

    view = MapViewOfFile(
        mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        vSize);

    if (view == NULL)
    {
        ZF_LOGF("MapViewOfFile has failed.");
    }

    return (struct tap_state*)view;
}

void freeMappingPtr(struct tap_state* m)
{
    (void) m;

    if (UnmapViewOfFile(view)    == 0 ||
        CloseHandle(mapping)     == 0 ||
        CloseHandle(fileHandle)  == 0)
    {
        ZF_LOGF("Error freeing memory map.");
    }
}

#endif
