#include "util.h"
#include "tap_state.h"

#include <stdbool.h>
#include <stdio.h>

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
    snprintf(buf, bufferSize, "%02d:%02d:%02d", m, s, ms);
}

void formatTimeToSeconds(char* buf, size_t bufferSize, int frames)
{
    float time = frames / 60.0f;

    bool neg = time < 0.0f;
    if (neg)
        time = -time;

    int ms = (int)(time * 100) % 100;
    int s  = (int)time % 100;
    snprintf(buf, bufferSize, "%c%02d:%02d", neg ? '-' : '+', s, ms);
}
