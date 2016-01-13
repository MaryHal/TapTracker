#ifndef WINDOW_H
#define WINDOW_H

#include "layout.h"

struct GLFWwindow;

struct window_t
{
        struct GLFWwindow* handle;
        unsigned int width;
        unsigned int height;
        struct layout_container_t layout;
};

void setupOpenGL(struct window_t* window, const unsigned int width, const unsigned int height);

struct window_t createWindow(unsigned int width, unsigned int height,
                             const char* title,
                             struct window_t* parent);

void destroyWindow(struct window_t* window);

void drawWindowLayout(struct window_t* window, struct draw_data_t* data);

#endif /* WINDOW_H */
