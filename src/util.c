#include "util.h"
#include "tap_state.h"

#include <stdbool.h>
#include <stdio.h>

#include <assert.h>
#include <zf_log.h>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))

#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

bool fileExists(const char* filename)
{
    if (access(filename, F_OK) != -1)
    {
        return true;
    }

    return false;
}

FILE* createOrOpenFile(const char* filename)
{
    int fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        if (errno == EEXIST)
        {
            ZF_LOGV("File \"%s\" does not exist. Attempting to create it...", filename);
        }
    }

    return fdopen(fd, "w");
}

#else

bool fileExists(const char* filename)
{
    FILE* f = fopen(filename, "rb");

    if (f == NULL)
    {
        return false;
    }

    fclose(file);
    return true;
}

FILE* createOrOpenFile(const char* filename)
{
    FILE* f = fopen(filename, "rb");

    if (f == NULL)
    {
        ZF_LOGV("File \"%s\" does not exist. Attempting to create it...", filename);

        fclose(file);
        return fopen(filename, "wb");
    }

    return f;
}

#endif

float frameTimeToSeconds(int frames)
{
    return frames / TIMER_FPS;
}

int frameTime(float seconds)
{
    return (seconds * TIMER_FPS);
}

void formatTimeToMinutes(char* buf, size_t bufferSize, int frames)
{
    float time = frames / 60.0f;
    int ms = (int)(time * 100) % 100;
    int m  = time / 60;
    int s  = (int)time % 60;
    snprintf(buf, bufferSize, "%02d:%02d.%02d", m, s, ms);
}

void formatTimeToSeconds(char* buf, size_t bufferSize, int frames)
{
    float time = frames / 60.0f;

    bool neg = time < 0.0f;
    if (neg)
        time = -time;

    int ms = (int)(time * 100) % 100;
    int s  = (int)((time / 100) * 100);
    snprintf(buf, bufferSize, "%c%d.%02d", neg ? '-' : '+', s, ms);
}
