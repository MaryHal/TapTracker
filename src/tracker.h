#ifndef TRACKER_H
#define TRACKER_H

#include <stdbool.h>
#include <stdint.h>

struct tap_state;

bool runTracker(struct tap_state* dataPtr);

#endif /* TRACKER_H */
