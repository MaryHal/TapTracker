#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

#include "game.h"
#include "font.h"
#include "draw.h"

#include "history.h"
#include "joystick.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("This program expects shmupmametgm as its first argument. Each subsequent argument is passed to shmupmametgm.");
        return 1;
    }

    int outfd[2];
    int infd[2];

    int oldstdin, oldstdout;

    pipe(outfd); // Where the parent is going to write to
    pipe(infd);  // From where parent is going to read

    oldstdin = dup(0); // Save current stdin
    oldstdout = dup(1); // Save stdout

    close(0);
    close(1);

    dup2(outfd[0], 0); // Make the read end of outfd pipe as stdin
    dup2(infd[1], 1); // Make the write end of infd as stdout

    int subprocessPid = fork();
    if (subprocessPid < 0) // Uh oh
    {
        perror("Could not fork process.");
        exit(1);
    }
    else if (subprocessPid == 0) // Child
    {
        // Child doesn't need to receive input from us (the parent).
        close(outfd[0]);
        close(outfd[1]);
        close(infd[0]);
        close(infd[1]);

        // Execute MAME. argv is guaranteed to be NULL-terminated, very
        // convenient.
        execv(argv[1], argv + 1);
    }
    else // Parent
    {
        // Restore the original std fds of parent
        close(0);
        close(1);
        dup2(oldstdin, 0);
        dup2(oldstdout, 1);

        close(outfd[0]); // These are being used by the child
        close(infd[1]);

        const int bufsize = 256;
        char buffer[bufsize];

        int incomingFD = infd[0];

        if (!glfwInit())
        {
            perror("Could not initialize GLFW.");
        }

        GLFWwindow* window;
        float ratio = 540 / 480.0;
        const unsigned int width = 960 - 640 * ratio;
        const unsigned int height = 540;

        {
            glfwWindowHint(GLFW_RESIZABLE, false);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

            window = glfwCreateWindow(width, height,
                                      "TapTracker",
                                      NULL,
                                      NULL);
            if (window == NULL)
            {
                perror("Could not create GLFW window.");
            }

            glfwMakeContextCurrent(window);

            setupOpenGL(width, height);
        }

        struct font_t font;
        loadFont(&font, "/usr/share/fonts/TTF/DroidSansFallback.ttf", 14.0f);

        struct history_t history;
        createHistory(&history);

        struct joystick_t joystick;
        createJoystick(&joystick, GLFW_JOYSTICK_1);

        struct game_t game;
        createNewGame(&game);

        #define SCALE_COUNT 2
        int scaleIndex = 0;
        float scales[SCALE_COUNT] = { 45.0f, 60.0f };

        while (!glfwWindowShouldClose(window))
        {
            // Check if child process has ended.
            int status = 0;
            if (waitpid(subprocessPid, &status, WNOHANG) != 0)
            {
                break;
            }

            // Read input from child process
            buffer[read(incomingFD, buffer, bufsize)] = 0;

            char* buf = buffer;
            char* search = NULL;
            while ((search = strchr(buf, '\n')) != NULL)
            {
                game.prevState = game.state;
                game.prevLevel = game.level;
                game.prevTime  = game.time;
                sscanf(search, "%d%d%d", &game.state, &game.level, &game.time);

                if (game.time < game.prevTime)
                {
                    perror("Time Error!");
                    printGameState(&game);
                }

                if (isInPlayingState(game.state) && game.level - game.prevLevel > 0)
                {
                    // Push a data point based on the newly acquired game state.
                    pushCurrentState(&game);
                }

                if (game.prevState != ACTIVE && game.state == ACTIVE)
                {
                    pushHistoryElement(&history, game.level);
                }

                // Reset if we were looking at the game over screen and just
                // moved to an idle state.
                if (game.prevState == GAMEOVER && !isInPlayingState(game.state))
                {
                    resetGame(&game);
                    resetHistory(&history);
                }

                // Discard the part of the string buffer that we already
                // processed.
                buf = search + 1;
            }
            glfwPollEvents();

            updateButtons(&joystick);
            if (buttonChange(&joystick, BUTTON_D) && getButtonState(&joystick, BUTTON_D) == GLFW_PRESS)
            {
                scaleIndex++;
            }
            pushCharFromJoystick(&history, &joystick);

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            float margin = 16.0f;
            drawSectionGraph(&game, &font, scales[scaleIndex % SCALE_COUNT], 0, 0, width, height / 2, margin);
            drawSectionLineCount(&game, &font, 70.0f, height / 2, margin);
            drawHistory(&history, &font, 0, height / 2 + 20.0f, margin);

            glfwSwapBuffers(window);
        }

        destroyGame(&game, false);
        destroyJoystick(&joystick, false);
        destroyHistory(&history, false);
        destroyFont(&font, false);

        glfwDestroyWindow(window);
        glfwTerminate();

        // Wait for child to finish
        printf("Parent process completed. Waiting for child to exit...\n");
        int childStatus = 0;
        wait(&childStatus);
    }

    return 0;
}
