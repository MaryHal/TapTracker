#include "tracker.h"

#include "game.h"
#include "font.h"
#include "draw.h"

#include "joystick.h"
#include "inputhistory.h"
#include "colortheme.h"

#include "layout.h"

#include <stdio.h>

#include <GLFW/glfw3.h>

bool runTracker(struct tap_state* dataPtr, unsigned int width, unsigned int height)
{
    if (glfwInit() == GL_FALSE)
    {
        perror("Could not initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = NULL;
    GLFWwindow* subwindow = NULL;

    window = glfwCreateWindow(width, height,
                              "TapTracker",
                              NULL,
                              NULL);

    glfwMakeContextCurrent(window);
    setupOpenGL(width, height);

    subwindow = glfwCreateWindow(128, 100,
                                 "TapTracker ButtonSpectrum",
                                 NULL,
                                 window);

    glfwMakeContextCurrent(subwindow);
    setupOpenGL(128, 100);

    if (window == NULL || subwindow == NULL)
    {
        perror("Could not create GLFW window");
        return false;
    }


    /* // Load then create bitmap font. */
    /* struct font_t* font = loadTTF(NULL, "/usr/share/fonts/TTF/PP821/PragmataPro.ttf", 13.0f); */
    /* exportBitmap("PP.png", font); */
    /* exportFontData("PP.bin", font); */

    /* struct font_t* backupfont = loadTTF(NULL, "/usr/share/fonts/TTF/DroidSans.ttf", 13.0f); */

    struct font_t* font = loadBitmapFontFiles(NULL, "PP.png", "PP.bin");

    struct joystick_t* joystick = createJoystick(NULL, GLFW_JOYSTICK_1);
    struct game_t* game = createNewGame(NULL);
    struct history_t* history = createHistory(NULL);

    struct button_spectrum_t* bspec = createButtonSheet(NULL);

    const int SCALE_COUNT = 2;
    float scales[] = { 45.0f, 60.0f };
    int scaleIndex = 0;

    struct layout_container_t* mainLayout = createLayoutContainer(NULL, width, height, 14.0f, 2.0f);
    struct layout_container_t* subLayout  = createLayoutContainer(NULL, 128, 100, 14.0f, 2.0f);

    addToContainerRatio(mainLayout, &drawSectionGraph, 0.75f);
    addToContainerRatio(mainLayout, &drawSectionTable, 1.00f);

    addToContainerRatio(subLayout, &drawInputHistory, 1.00f);

    struct draw_data_t data =
    {
        .game = game,
        .font = font,
        .history = history,
        .bspec = bspec,
        .scale = scales[scaleIndex]
    };

    while (!glfwWindowShouldClose(window))
    {
        updateGameState(game, history, dataPtr);

        glfwPollEvents();

        updateButtons(joystick);
        if (buttonChangedToState(joystick, BUTTON_D, GLFW_PRESS))
        {
            scaleIndex++;
            data.scale = scales[scaleIndex % SCALE_COUNT];
        }

        // Update input history
        pushCharFromJoystick(history, joystick);

        glfwMakeContextCurrent(window);
        setGLClearColor();
        glClear(GL_COLOR_BUFFER_BIT);
        drawLayout(mainLayout, &data, false);
        glfwSwapBuffers(window);

        glfwMakeContextCurrent(subwindow);
        setGLClearColor();
        glClear(GL_COLOR_BUFFER_BIT);
        drawLayout(subLayout, &data, false);
        glfwSwapBuffers(subwindow);
    }

    destroyButtonSheet(bspec, true);
    destroyHistory(history, true);
    destroyContainer(mainLayout, true);
    destroyContainer(subLayout, true);
    destroyGame(game, true);
    destroyJoystick(joystick, true);
    destroyFont(font, true);

    glfwDestroyWindow(window);
    glfwDestroyWindow(subwindow);
    glfwTerminate();

    return true;
}
