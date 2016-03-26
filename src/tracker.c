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

#include <stdio.h>

#include <GLFW/glfw3.h>

#define INCBIN_PREFIX g_
#include <incbin.h>
INCBIN(PPImage, "bin/PP.png");
INCBIN(PPData, "bin/PP.bin");

bool runTracker(struct tap_state* dataPtr, struct tracker_settings_t settings)
{
    if (glfwInit() == GL_FALSE)
    {
        perror("Could not initialize GLFW");
        return false;
    }

    struct window_t* mainWindow = window_create("TapTracker Graph", 240, 540, NULL);

    layout_init(&mainWindow->layout, mainWindow->width, mainWindow->height, 14.0f, 2.0f);

    addToContainerRatio(&mainWindow->layout, &drawSectionGraph, 0.72f);
    addToContainerFixed(&mainWindow->layout, &drawSectionTableOverall, 130.0f);
    addToContainerFixed(&mainWindow->layout, &drawCurrentState, 14.0f);

    struct window_t* auxWindow = window_create("TapTracker Extras", 180, 120, mainWindow);

    layout_init(&auxWindow->layout, auxWindow->width, auxWindow->height, 4.0f, 2.0f);
    addToContainerRatio(&auxWindow->layout, &drawGameHistory, 1.0f);

    struct window_t* subWindow = NULL;

    if (settings.enableJoystick)
    {
        subWindow = window_create("TapTracker ButtonSpectrum", 180, 112, mainWindow);
        layout_init(&subWindow->layout, subWindow->width, subWindow->height, 4.0f, 0.0f);
        addToContainerRatio(&subWindow->layout, &drawInputHistory, 1.00f);
    }

    struct window_t* windows[3] =
    {
        mainWindow,
        auxWindow,
        subWindow
    };
    size_t windowCount = 3;

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
    struct input_history_t* history = NULL;

    if (settings.enableJoystick)
    {
        joystick = joystick_create(GLFW_JOYSTICK_1, settings.jmap);
        history = input_history_create();
    }

    struct button_spectrum_t* bspec = button_spectrum_create();
    struct section_table_t* table = section_table_create();

    struct game_history_t* gh = game_history_create();

    struct draw_data_t data =
    {
        .game = game,
        .font = &font,
        .history = history,
        .bspec = bspec,
        .table = table,
        .gh = gh,
        .scale = 60.0f
    };

    while (!windowSetShouldClose(windows, windowCount))
    {
        updateGameState(game, history, table, gh, dataPtr);

        if (game->curState.gameMode == TAP_MODE_DEATH)
            data.scale = 45.0f;
        else
            data.scale = 60.0f;

        glfwPollEvents();

        // Update input history
        if (settings.enableJoystick && joystick)
        {
            updateButtons(joystick);
            pushInputFromJoystick(history, joystick);
        }

        drawWindowSet(windows, windowCount, &data);
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

    if (mainWindow)
        window_destroy(mainWindow);

    if (auxWindow)
        window_destroy(auxWindow);

    if (subWindow)
        window_destroy(subWindow);

    glfwTerminate();

    return true;
}
