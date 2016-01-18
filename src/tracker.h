#ifndef TRACKER_H
#define TRACKER_H

#include <stdbool.h>
#include <stdint.h>

#include "joystick.h"

struct tap_state;

struct tracker_settings_t
{
        bool enableJoystick;
        struct joystick_mapping_t jmap;
};

bool runTracker(struct tap_state* dataPtr, struct tracker_settings_t settings);

#endif /* TRACKER_H */
