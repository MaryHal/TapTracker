#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

float frameTimeToSeconds(int frames);
int frameTime(float seconds);

void formatTimeToMinutes(char* buf, size_t bufferSize, int frames);
void formatTimeToSeconds(char* buf, size_t bufferSize, int frames); 

#endif /* UTIL_H */
