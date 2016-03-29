#ifndef TRACKER_H
#define TRACKER_H

#include <stdbool.h>

struct tap_state;

bool runTracker(struct tap_state* dataPtr, int argc, const char* argv[]);

#endif /* TRACKER_H */
