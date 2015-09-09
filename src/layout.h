#ifndef LAYOUT_H
#define LAYOUT_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_LAYOUT_ELEMENTS 4

struct game_t;
struct font_t;
struct layout_element_t
{
        float x;
        float y;
        float width;
        float height;

        void (*drawFunc)(struct game_t* game, struct font_t* font, float width, float height);
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
void addToContainerRatio(struct layout_container_t* container,
                         void (*drawFunc)(struct game_t* game, struct font_t* font, float width, float height),
                         float ratio);

void addToContainerFixed(struct layout_container_t* container,
                         void (*drawFunc)(struct game_t* game, struct font_t* font, float width, float height),
                         float pixelHeight);

void drawLayout(struct layout_container_t* container, struct game_t* game, struct font_t* font);

#endif /* LAYOUT_H */
