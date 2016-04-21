#include "tracker.h"

#include "draw.h"

#include "game.h"
#include "font.h"

#include "joystick.h"
#include "inputhistory.h"
#include "buttonquads.h"

#include "sectiontable.h"
#include "gamehistory.h"

#include "window.h"
#include "layout.h"

#include "config.h"

#include <stdio.h>
#include <zf_log.h>

#include <GLFW/glfw3.h>

#include <flag.h>

#define VERSION "v2.0"

#define INCBIN_PREFIX g_
#include <incbin.h>
INCBIN(PPImage, "src/bin/PP.png");
INCBIN(PPData,  "src/bin/PP.bin");

bool runTracker(struct tap_state* dataPtr, int argc, const char* argv[])
{
    if (glfwInit() == GL_FALSE)
    {
        ZF_LOGF("Could not initialize GLFW.");
        return false;
    }

    int arg_enableJoystick = -1;
    const char* config_path = DEFAULT_CONFIG_FILENAME;
    const char* pb_path = DEFAULT_GOLD_ST_FILENAME;

    flag_int(&arg_enableJoystick, "js", "Set to 1 to enable joystick support. This setting has priority over the config file.");
    flag_str(&config_path, "config", "Set json config file path");
    flag_str(&pb_path, "pb", "Set \"Personal Bests\" file path");
    flag_parse(argc, argv, VERSION);

    loadConfig(config_path);

    if (arg_enableJoystick == 1)
    {
        tt_config.enableJoystick = true;
    }

    /* // Load then export bitmap font. */
    /* struct font_t* font = loadTTF(font_create(), "/usr/share/fonts/TTF/PP821/PragmataPro.ttf", 13.0f); */
    /* exportBitmap("PP.png", font); */
    /* exportFontData("PP.bin", font); */

    /* struct font_t* backupfont = loadTTF(font_create(), "/usr/share/fonts/TTF/DroidSans.ttf", 13.0f); */

    /* loadBitmapFontFiles(&font, "PP.png", "PP.bin"); */

    struct font_t font;
    font_init(&font);
    loadBitmapFontData(&font,
                       g_PPImageData, g_PPImageSize,
                       g_PPDataData, g_PPDataSize);

    struct game_t* game = game_create();

    struct joystick_t* joystick = NULL;
    struct input_history_t* history = input_history_create();

    if (tt_config.enableJoystick)
    {
        joystick = joystick_create(GLFW_JOYSTICK_1, tt_config.jmap);
    }

    struct button_spectrum_t* bspec = button_spectrum_create();
    struct section_table_t* table = section_table_create(pb_path);

    struct game_history_t* gh = game_history_create();

    struct draw_data_t data =
        {
            .game = game,
            .font = &font,
            .history = history,
            .bspec = bspec,
            .table = table,
            .gh = gh
        };

    bindDrawData(data);

    while (!windowSetShouldClose(tt_config.windowset, tt_config.windowCount))
    {
        updateGameState(game, history, table, gh, dataPtr);

        glfwPollEvents();

        // Update input history
        if (tt_config.enableJoystick && joystick)
        {
            updateButtons(joystick);
            pushInputFromJoystick(history, joystick);
        }

        drawWindowSet(tt_config.windowset, tt_config.windowCount);
    }

    game_history_destroy(gh);
    section_table_destroy(table);

    button_spectrum_destroy(bspec);
    game_destroy(game);
    font_terminate(&font);

    if (history)
        input_history_destroy(history);

    if (joystick)
        joystick_destroy(joystick);

    windowSet_destroy(tt_config.windowset, tt_config.windowCount);

    glfwTerminate();

    return true;
}

