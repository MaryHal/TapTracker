#include "window.h"
#include "colortheme.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

void setupOpenGL(struct window_t* window, const unsigned int width, const unsigned int height)
{
    glfwMakeContextCurrent(window->handle);

    // OpenGL 2d perspective
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

    // Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    /* glDisable(GL_LIGHTING); */
    /* glDisable(GL_DEPTH_TEST); */

    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);
}

struct window_t* createWindow(unsigned int width, unsigned int height,
                             const char* title,
                             struct window_t* parent)
{
    struct window_t* window = malloc(sizeof(struct window_t));

    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window->handle = glfwCreateWindow(width, height,
                                      title,
                                      NULL,
                                      (parent == NULL || parent->handle == NULL) ? NULL : parent->handle);
    window->width = width;
    window->height = height;

    if (window->handle == NULL)
    {
        perror("Could not create GLFW window");
        return NULL;
    }

    setupOpenGL(window, width, height);

    return window;
}

void destroyWindow(struct window_t* window)
{
    glfwDestroyWindow(window->handle);
    destroyContainer(&window->layout, false);

    free(window);
}

void drawWindowLayout(struct window_t* window, struct draw_data_t* data)
{
    glfwMakeContextCurrent(window->handle);

    setGLClearColor();
    glClear(GL_COLOR_BUFFER_BIT);

    drawLayout(&window->layout, data, false);

    glfwSwapBuffers(window->handle);
}
