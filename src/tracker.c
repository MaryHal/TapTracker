#include "tracker.h"

#include "game.h"
#include "font.h"
#include "draw.h"

#include "joystick.h"
#include "colortheme.h"

#include "layout.h"

#include <sys/wait.h>

#include <stdio.h>

#include <GLFW/glfw3.h>

bool runTracker(int32_t* dataPtr, unsigned int width, unsigned int height)
{
    if (!glfwInit())
    {
        perror("Could not initialize GLFW");
        return false;
    }

    GLFWwindow* window;

    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(width, height,
                              "TapTracker",
                              NULL,
                              NULL);
    if (window == NULL)
    {
        perror("Could not create GLFW window");
        return false;
    }

    glfwMakeContextCurrent(window);

    setupOpenGL(width, height);

    struct font_t font;
    /* loadTTF(&font, "/usr/share/fonts/TTF/PP821/PragmataPro.ttf", 13.0f); */
    loadBitmapFont(&font, "PP.png", "PP.bin");

    struct joystick_t joystick;
    createJoystick(&joystick, GLFW_JOYSTICK_1);

    struct game_t game;
    createNewGame(&game);

    const int SCALE_COUNT = 2;
    float scales[] = { 45.0f, 60.0f };
    int scaleIndex = 0;

    struct layout_container_t layout;
    createLayoutContainer(&layout, width, height, 14.0f, 2.0f);

    addToContainerRatio(&layout, &drawSectionGraph, 0.75f);
    addToContainerRatio(&layout, &drawSectionTable, 1.00f);
    /* addToContainerRatio(&layout, &drawInputHistory, 1.00f); */

    while (!glfwWindowShouldClose(window))
    {
        updateGameState(&game, dataPtr);

        glfwPollEvents();

        updateButtons(&joystick);
        if (buttonChangedToState(&joystick, BUTTON_D, GLFW_PRESS))
        {
            scaleIndex++;
        }
        pushCharFromJoystick(&game.inputHistory, &joystick);

        setGLClearColor();
        glClear(GL_COLOR_BUFFER_BIT);

        drawLayout(&layout, &game, &font, &scales[scaleIndex % SCALE_COUNT]);

        glfwSwapBuffers(window);
    }

    destroyGame(&game, false);
    destroyJoystick(&joystick, false);
    destroyFont(&font, false);

    glfwDestroyWindow(window);
    glfwTerminate();

    return true;
}
