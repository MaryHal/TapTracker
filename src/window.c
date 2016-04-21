#include "window.h"
#include "colortheme.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <zf_log.h>

void window_init(struct window_t* w,
                 const char* title,
                 const unsigned int width, const unsigned int height,
                 struct window_t* parent)
{
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* sharedContext = (parent == NULL) ? NULL : parent->handle;

    w->handle = glfwCreateWindow(width, height,
                                 title,
                                 NULL,
                                 sharedContext);
    w->width = width;
    w->height = height;

    if (w->handle == NULL)
    {
        ZF_LOGF("Could not create GLFW window.");
    }

    setupOpenGL(w, width, height);
}

void window_terminate(struct window_t* w)
{
    glfwDestroyWindow(w->handle);
    layout_terminate(&w->layout);
}

struct window_t* window_create(const char* title,
                               const unsigned int width, const unsigned int height,
                               struct window_t* parent)
{
    struct window_t* w = malloc(sizeof(struct window_t));
    if (w)
    {
        window_init(w, title, width, height, parent);
    }

    return w;
}

void window_destroy(struct window_t* w)
{
    window_terminate(w);
    free(w);
}

void setupOpenGL(struct window_t* window, const unsigned int width, const unsigned int height)
{
    glfwMakeContextCurrent(window->handle);

    // OpenGL 2d perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
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

void drawWindowLayout(struct window_t* window)
{
    glfwMakeContextCurrent(window->handle);

    setGLClearColor();
    glClear(GL_COLOR_BUFFER_BIT);

    drawLayout(&window->layout, false);

    glfwSwapBuffers(window->handle);
}

bool windowSetShouldClose(struct window_t** windows, size_t windowCount)
{
    for (size_t i = 0; i < windowCount; ++i)
    {
        struct window_t* w = windows[i];

        if (w)
        {
            if (glfwWindowShouldClose(windows[i]->handle))
                return true;
        }
    }

    return false;
}

void drawWindowSet(struct window_t** windows, size_t windowCount)
{
    for (size_t i = 0; i < windowCount; ++i)
    {
        struct window_t* w = windows[i];

        if (w)
            drawWindowLayout(w);
    }
}

void windowSet_destroy(struct window_t** windowset, size_t windowCount)
{
    for (size_t i = 0; i < windowCount; ++i)
    {
        window_destroy(windowset[i]);
    }
}
