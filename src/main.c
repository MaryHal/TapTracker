#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

#include "game.h"
#include "draw.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("This program expects shmupmametgm as its first argument. Each subsequent argument is passed to shmupmametgm.");
        return 1;
    }
    else
    {
        for (int i = 0; i < argc; ++i)
        {
            printf("%s\n", argv[i]);
        }
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

        // Execute MAME.
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

        struct game_t game;
        createNewGame(&game);

        while (true)
        {
            // Check if child process has ended.
            int status = 0;
            if (waitpid(subprocessPid, &status, WNOHANG) != 0)
            {
                break;
            }

            buffer[read(incomingFD, buffer, bufsize)] = 0;

            int state, level, time;

            char* buf = buffer;
            char* search = NULL;
            while ((search = strchr(buf, '\n')) != NULL)
            {
                sscanf(search, "%d%d%d", &state, &level, &time);

                /* printf("%d %d %d\n", state, level, time); */

                if (isInPlayingState(state))
                {
                    pushDataPoint(&game, (struct datapoint_t){ level, time });
                }
                buf = search + 1;
            }

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            drawSectionGraph(&game, 8, 8, width - 16, height - 16);

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
