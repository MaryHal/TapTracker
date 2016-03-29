#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stddef.h>

#include "joystick.h"

struct window_t;

struct config_t
{
    bool enableJoystick;
    struct joystick_mapping_t jmap;

    struct window_t** windowset;
    size_t windowCount;
};

extern const char* DEFAULT_CONFIG_FILENAME;
extern struct config_t tt_config;

void loadConfig(const char* filename);

#endif /* CONFIG_H */
