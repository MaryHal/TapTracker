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
        const unsigned int width = 960 - 640;
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
        loadFont(&font, "/usr/share/fonts/TTF/DroidSansFallback.ttf", 20.0f);

        struct game_t game;
        createNewGame(&game);

        while (!glfwWindowShouldClose(window))
        {
            // Check if child process has ended.
            int status = 0;
            if (waitpid(subprocessPid, &status, WNOHANG) != 0)
            {
                break;
            }

            buffer[read(incomingFD, buffer, bufsize)] = 0;

            char* buf = buffer;
            char* search = NULL;
            while ((search = strchr(buf, '\n')) != NULL)
            {
                game.prevState = game.state;
                game.prevLevel = game.level;
                game.prevTime  = game.time;
                sscanf(search, "%d%d%d", &game.state, &game.level, &game.time);

                /* printf("%d %d %d\n", state, level, time); */

                if (isInPlayingState(game.state))
                {
                    pushDataPoint(&game, (struct datapoint_t){ game.level, game.time });
                }

                // Reset if we were looking at the game over screen and just
                // moved to an idle state.
                if (game.prevState == GAMEOVER && !isInPlayingState(game.state))
                {
                    resetGame(&game);
                }

                // Discard the part of the string buffer that we already
                // processed.
                buf = search + 1;
            }

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            drawSectionGraph(&game, &font, 16, 16, width - 32, height - 32);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();

        // Wait for child to finish
        printf("Parent process completed. Waiting for child to exit...\n");
        int childStatus = 0;
        wait(&childStatus);
    }

    return 0;
}
