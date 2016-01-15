#include "tracker.h"

#include "game.h"
#include "font.h"
#include "draw.h"

#include "history.h"
#include "sectiontable.h"

#include "joystick.h"
#include "buttonquads.h"

#include "window.h"
#include "layout.h"

#include <stdio.h>

#include <GLFW/glfw3.h>

#define INCBIN_PREFIX g_
#include <incbin.h>
INCBIN(PPImage, "bin/PP.png");
INCBIN(PPData, "bin/PP.bin");

bool runTracker(struct tap_state* dataPtr)
{
    if (glfwInit() == GL_FALSE)
    {
        perror("Could not initialize GLFW");
        return false;
    }

    struct window_t mainWindow = createWindow(240, 540, "TapTracker Graph", NULL);
    createLayoutContainer(&mainWindow.layout, mainWindow.width, mainWindow.height, 14.0f, 2.0f);
    addToContainerRatio(&mainWindow.layout, &drawSectionGraph, 0.75f);
    /* addToContainerRatio(&mainWindow.layout, &drawSectionTable, 1.00f); */
    addToContainerRatio(&mainWindow.layout, &drawSectionTableOverall, 1.00f);

    struct window_t subWindow = createWindow(96, 112, "TapTracker ButtonSpectrum", &mainWindow);
    createLayoutContainer(&subWindow.layout, subWindow.width, subWindow.height, 4.0f, 0.0f);
    addToContainerRatio(&subWindow.layout, &drawInputHistory, 1.00f);

    /* // Load then create bitmap font. */
    /* struct font_t* font = loadTTF(NULL, "/usr/share/fonts/TTF/PP821/PragmataPro.ttf", 13.0f); */
    /* exportBitmap("PP.png", font); */
    /* exportFontData("PP.bin", font); */

    /* struct font_t* backupfont = loadTTF(NULL, "/usr/share/fonts/TTF/DroidSans.ttf", 13.0f); */

    struct font_t font;
    /* loadBitmapFontFiles(&font, "PP.png", "PP.bin"); */
    loadBitmapFontData(&font, g_PPImageData, g_PPImageSize, g_PPDataData, g_PPDataSize);

    struct joystick_t joystick;
    createJoystick(&joystick, GLFW_JOYSTICK_1);

    struct game_t game;
    createNewGame(&game);

    struct history_t history;
    createHistory(&history);

    struct button_spectrum_t bspec;
    createButtonSpriteSheet(&bspec);

    struct section_table_t table;
    section_table_init(&table);

    struct draw_data_t data =
    {
        .game = &game,
        .font = &font,
        .history = &history,
        .bspec = &bspec,
        .table = &table,
        .scale = 60.0f
    };

    while (!glfwWindowShouldClose(mainWindow.handle) &&
           !glfwWindowShouldClose(subWindow.handle))
    {
        updateGameState(&game, &history, &table, dataPtr);

        if (game.curState.gameMode == TAP_MODE_DEATH)
            data.scale = 45.0f;
        else
            data.scale = 60.0f;

        glfwPollEvents();

        // Update input history
        updateButtons(&joystick);
        pushCharFromJoystick(&history, &joystick);

        drawWindowLayout(&mainWindow, &data);
        drawWindowLayout(&subWindow, &data);
    }

    section_table_terminate(&table);

    destroyButtonSpriteSheet(&bspec, false);
    destroyHistory(&history, false);
    destroyGame(&game, false);
    destroyJoystick(&joystick, false);
    destroyFont(&font, false);

    destroyWindow(&mainWindow);
    destroyWindow(&subWindow);

    glfwTerminate();

    return true;
}
