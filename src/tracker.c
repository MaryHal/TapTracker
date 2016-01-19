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

#include <glad/glad.h>
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

    struct window_t mainWindow = createWindow(240, 540, "TapTracker Graph", NULL);
    createLayoutContainer(&mainWindow.layout, mainWindow.width, mainWindow.height, 14.0f, 2.0f);
    addToContainerRatio(&mainWindow.layout, &drawSectionGraph, 0.72f);
    addToContainerFixed(&mainWindow.layout, &drawSectionTableOverall, 130.0f);
    addToContainerFixed(&mainWindow.layout, &drawCurrentState, 14.0f);
    /* addToContainerRatio(&mainWindow.layout, &drawSectionTable, 1.00f); */
    /* addToContainerRatio(&mainWindow.layout, &drawSectionTableOverall, 1.00f); */

    struct window_t subWindow = createWindow(96, 112, "TapTracker ButtonSpectrum", &mainWindow);
    createLayoutContainer(&subWindow.layout, subWindow.width, subWindow.height, 4.0f, 0.0f);
    addToContainerRatio(&subWindow.layout, &drawInputHistory, 1.00f);

    /* // Load then create bitmap font. */
    /* struct font_t* font = loadTTF(NULL, "/usr/share/fonts/TTF/PP821/PragmataPro.ttf", 13.0f); */
    /* exportBitmap("PP.png", font); */
    /* exportFontData("PP.bin", font); */

    /* struct font_t* backupfont = loadTTF(NULL, "/usr/share/fonts/TTF/DroidSans.ttf", 13.0f); */
    /* loadBitmapFontFiles(&font, "PP.png", "PP.bin"); */

    struct font_t* font = loadBitmapFontData(NULL, g_PPImageData, g_PPImageSize, g_PPDataData, g_PPDataSize);

    struct game_t* game = createNewGame(NULL);

    struct joystick_t* joystick = NULL;
    struct input_history_t* history = NULL;

    if (settings.enableJoystick)
    {
        joystick = createJoystick(NULL, GLFW_JOYSTICK_1, settings.jmap);
        history = createInputHistory(NULL);
    }

    struct button_spectrum_t* bspec = createButtonSpriteSheet(NULL);
    struct section_table_t* table = section_table_create();

    struct game_history_t* gh = game_history_create();

    struct draw_data_t data =
    {
        .game = game,
        .font = font,
        .history = history,
        .bspec = bspec,
        .table = table,
        .gh = gh,
        .scale = 60.0f
    };

    while (!glfwWindowShouldClose(mainWindow.handle) &&
           !glfwWindowShouldClose(subWindow.handle))
    {
        updateGameState(game, history, table, gh, dataPtr);

        if (game->curState.gameMode == TAP_MODE_DEATH)
            data.scale = 45.0f;
        else
            data.scale = 60.0f;

        glfwPollEvents();

        drawWindowLayout(&mainWindow, &data);

        // Update input history
        if (settings.enableJoystick && joystick)
        {
            updateButtons(joystick);
            pushInputFromJoystick(history, joystick);
            drawWindowLayout(&subWindow, &data);
        }
    }

    section_table_destroy(table);

    destroyButtonSpriteSheet(bspec, true);
    destroyInputHistory(history, true);
    destroyGame(game, true);
    destroyJoystick(joystick, true);
    destroyFont(font, true);

    destroyWindow(&mainWindow);
    destroyWindow(&subWindow);

    glfwTerminate();

    return true;
}
