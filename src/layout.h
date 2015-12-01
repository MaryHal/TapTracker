#ifndef LAYOUT_H
#define LAYOUT_H

#include <stddef.h>
#include <stdbool.h>

#include "draw.h"

#define MAX_LAYOUT_ELEMENTS 4

typedef void (*draw_function_p)(struct draw_data_t* data, float width, float height);

struct layout_element_t
{
        float x;
        float y;
        float width;
        float height;

        draw_function_p drawFunc;
};

// Vertical only for now
struct layout_container_t
{
        struct layout_element_t elements[MAX_LAYOUT_ELEMENTS];
        size_t size;

        float outerMargin;
        float innerMargin;
        float leftoverWidth;
        float leftoverHeight;
};

struct layout_container_t* createLayoutContainer(struct layout_container_t* c,
                                                 float width, float height,
                                                 float outerMargin, float innerMargin);
void destroyContainer(struct layout_container_t* c, bool freeMe);

// Add a draw function to the layout and make sure it fills (ratio)% of the remaining area.
void addToContainerRatio(struct layout_container_t* container, draw_function_p drawFunc,
                         float ratio);

// Add a draw function to the layout and ensure it's pixelHeight pixels tall.
void addToContainerFixed(struct layout_container_t* container, draw_function_p drawFunc,
                         float pixelHeight);

// Draw our layout container. If the debug flag is checked, this function will
// also draw borders for layout element.
void drawLayout(struct layout_container_t* container, struct draw_data_t* data_container, bool debug);

#endif /* LAYOUT_H */
