#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

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

    const char* sharedMemKey = "tgm2p_data";
    int fd = shm_open(sharedMemKey, O_RDWR | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
    size_t vsize = sizeof(int) * 4;

    // Stretch our new file to the suggested size.
    int result = lseek(fd, vsize-1, SEEK_SET);
    if (result == -1) {
	perror("Error calling lseek() to 'stretch' the file");
	exit(1);
    }

    /* Something needs to be written at the end of the file to
     * have the file actually have the new size.
     * Just writing an empty string at the current file position will do.
     *
     * Note:
     *  - The current position in the file is at the end of the stretched
     *    file due to the call to lseek().
     *  - An empty string is actually a single '\0' character, so a zero-byte
     *    will be written at the last byte of the file.
     */
    result = write(fd, "", 1);
    if (result != 1) {
	perror("Error writing last byte of the file");
	exit(1);
    }

    int* addr = mmap(NULL, vsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("Parent: Could not map memory");
        exit(1);
    }

    // Lock the mapped region into memory
    if(mlock(addr, vsize) != 0)
    {
        perror("mlock failure");
        exit(1);
    }

    int subprocessPid = fork();
    if (subprocessPid < 0) // Uh oh
    {
        perror("Could not fork process.");
        exit(1);
    }
    else if (subprocessPid == 0) // Child
    {
        // Execute MAME. argv is guaranteed to be NULL-terminated, very
        // convenient.
        execv(argv[1], argv + 1);
    }
    else // Parent
    {
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

            // Read input from child process and add it to the graph
            {
                game.prevState = game.state;
                game.prevLevel = game.level;
                game.prevTime  = game.time;

                game.state = addr[0];
                game.level = addr[1];
                game.time  = addr[2];

                if (isInPlayingState(game.state) &&
                    (game.time < game.prevTime || game.level < game.prevLevel))
                {
                    perror("Internal State Error");
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
            }

            glfwPollEvents();

            updateButtons(&joystick);
            if (buttonChangedToState(&joystick, BUTTON_D, GLFW_PRESS))
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

    int status = 0;
    status = munlock(addr, vsize);
    status = munmap(addr, vsize);      /* Unmap the page */
    status = close(fd);                /* Close file */
    status = shm_unlink(sharedMemKey); /* Unlink shared-memory object */

    return 0;
}
