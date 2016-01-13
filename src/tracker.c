#include "tracker.h"

#include "game.h"
#include "font.h"
#include "draw.h"

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
    addToContainerRatio(&mainWindow.layout, &drawSectionTable, 1.00f);

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
    createButtonSheet(&bspec);

    const int SCALE_COUNT = 2;
    float scales[] = { 45.0f, 60.0f };
    int scaleIndex = 0;

    struct draw_data_t data =
    {
        .game = &game,
        .font = &font,
        .history = &history,
        .bspec = &bspec,
        .scale = scales[scaleIndex]
    };

    while (!glfwWindowShouldClose(mainWindow.handle) &&
           !glfwWindowShouldClose(subWindow.handle))
    {
        updateGameState(&game, &history, dataPtr);

        glfwPollEvents();

        updateButtons(&joystick);
        if (buttonChangedToState(&joystick, BUTTON_D, GLFW_PRESS))
        {
            scaleIndex++;
            data.scale = scales[scaleIndex % SCALE_COUNT];
        }

        // Update input history
        pushCharFromJoystick(&history, &joystick);

        drawWindowLayout(&mainWindow, &data);
        drawWindowLayout(&subWindow, &data);
    }

    destroyButtonSheet(&bspec, false);
    destroyHistory(&history, false);
    destroyGame(&game, false);
    destroyJoystick(&joystick, false);
    destroyFont(&font, false);

    destroyWindow(&mainWindow);
    destroyWindow(&subWindow);

    glfwTerminate();

    return true;
}
