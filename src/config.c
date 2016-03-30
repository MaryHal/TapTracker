#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <assert.h>
#include <zf_log.h>

#include <parson.h>

#include "window.h"
#include "draw.h"

#include "util.h"

#define INCBIN_PREFIX g_
#include <incbin.h>
INCBIN(DEFAULT_CONFIG_FILE_CONTENTS_, "bin/default_config.json");

const char* DEFAULT_CONFIG_FILENAME = "config.json";
struct config_t tt_config =
{
    .enableJoystick = false,
    .jmap =
    {
        // Joystick  A  B  C
        .buttons = { 1, 2, 3 },
        .axisHori = 6,
        .axisVert = 7
    },
    .windowset = NULL,
    .windowCount = 0
};

void loadConfig(const char* filename)
{
    assert(filename != NULL);

    JSON_Value* root = NULL;

    // TODO: Fix the possible race condition here. If another process creates
    // this file after we check that it doesn't exist, bad news bears.
    if (!fileExists(filename))
    {
        ZF_LOGI("Config file @ \"%s\" not found, attempting to create it.", filename);

        FILE* f = createOrOpenFile(filename);

        unsigned int ret =
        fwrite(g_DEFAULT_CONFIG_FILE_CONTENTS_Data,
               sizeof(unsigned char),
               g_DEFAULT_CONFIG_FILE_CONTENTS_Size,
               f);

        assert(ret == g_DEFAULT_CONFIG_FILE_CONTENTS_Size);

        fclose(f);
    }

    ZF_LOGV("Reading config file @ \"%s\".", filename);

    root = json_parse_file_with_comments(filename);

    if (root == NULL)
    {
        ZF_LOGE("Error reading config file.");
        return;
    }

    if (json_value_get_type(root) != JSONObject)
    {
        ZF_LOGE("Unexpected root type for config file");
        return;
    }

    JSON_Object* configObject = json_value_get_object(root);

    JSON_Object* joystickSettings = json_object_get_object(configObject, "joystick");
    {
        tt_config.enableJoystick = json_object_get_boolean(joystickSettings, "enabled");
        tt_config.jmap.axisHori  = json_object_get_number(joystickSettings, "axis-hori");
        tt_config.jmap.axisVert  = json_object_get_number(joystickSettings, "axis-vert");

        tt_config.jmap.buttons[BUTTON_A] = json_object_get_number(joystickSettings, "button-a");
        tt_config.jmap.buttons[BUTTON_B] = json_object_get_number(joystickSettings, "button-b");
        tt_config.jmap.buttons[BUTTON_C] = json_object_get_number(joystickSettings, "button-c");
    }

    JSON_Array* windowArray = json_object_get_array(configObject, "windows");
    {
        size_t windowCount = json_array_get_count(windowArray);
        tt_config.windowCount = windowCount;
        tt_config.windowset = malloc(sizeof(struct window_t*) * windowCount);

        for (size_t i = 0; i < windowCount; ++i)
        {
            JSON_Object* windowSettings = json_array_get_object(windowArray, i);

            tt_config.windowset[i] = window_create(json_object_get_string(windowSettings, "name"),
                                                   json_object_get_number(windowSettings, "width"),
                                                   json_object_get_number(windowSettings, "height"),
                                                   i == 0 ? NULL : tt_config.windowset[0]);

            layout_init(&tt_config.windowset[i]->layout,
                        tt_config.windowset[i]->width,
                        tt_config.windowset[i]->height,
                        json_object_get_number(windowSettings, "inner-margin"),
                        json_object_get_number(windowSettings, "outer-margin"));

            JSON_Array* layoutArray = json_object_get_array(windowSettings, "layout");
            size_t layoutElementCount = json_array_get_count(layoutArray);

            for (size_t j = 0; j < layoutElementCount; ++j)
            {
                JSON_Object* layoutElementObject = json_array_get_object(layoutArray, j);

                const char* layoutElementType = json_object_get_string(layoutElementObject, "type");
                float ratioOrFixed = json_object_get_number(layoutElementObject, "ratio");

                if (ratioOrFixed <= 1.0f)
                {
                    addToContainerRatio(&tt_config.windowset[i]->layout,
                                        stringToDrawFunc(layoutElementType),
                                        ratioOrFixed);
                }
                else
                {
                    addToContainerFixed(&tt_config.windowset[i]->layout,
                                        stringToDrawFunc(layoutElementType),
                                        ratioOrFixed);
                }
            }
        }
    }

    json_value_free(root);
}
