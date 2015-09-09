#include "layout.h"

#include <stdlib.h>

#include <GLFW/glfw3.h>

struct layout_container_t* createLayoutContainer(struct layout_container_t* c,
                                                 float width, float height,
                                                 float outerMargin, float innerMargin)
{
    if (c == NULL)
    {
        c = (struct layout_container_t*)malloc(sizeof(struct layout_container_t));
    }

    c->outerMargin = outerMargin;
    c->innerMargin = innerMargin;

    c->leftoverWidth = width - 2 * outerMargin;
    c->leftoverHeight = height - 2 * outerMargin;

    return c;
}

void destroyContainer(struct layout_container_t* c, bool freeMe)
{
    if (freeMe)
        free(c);
}

void addToContainerRatio(struct layout_container_t* container,
                         void (*drawFunc)(struct game_t* game, struct font_t* font, float width, float height),
                         float ratio)
{
    addToContainerFixed(container, drawFunc, container->leftoverHeight * ratio);
}

void addToContainerFixed(struct layout_container_t* container,
                         void (*drawFunc)(struct game_t* game, struct font_t* font, float width, float height),
                         float pixelHeight)
{
    struct layout_element_t* prevElement = container->size > 0 ? &container->elements[container->size - 1] : NULL;

    container->elements[container->size] =
        (struct layout_element_t)
        {
            .x = 0.0f,
            .y = prevElement == NULL ? 0.0f : prevElement->y + prevElement->height,
            .width  = container->leftoverWidth,
            .height = pixelHeight,
            .drawFunc = drawFunc
        };

    container->leftoverHeight -= pixelHeight;

    container->size++;
}

void drawLayout(struct layout_container_t* container, struct game_t* game, struct font_t* font)
{
    glPushMatrix();
    glTranslatef(container->outerMargin, container->outerMargin, 0.0f);

    for (size_t i = 0; i < container->size; i++)
    {
        struct layout_element_t* e = &container->elements[i];

        float adjustedX = e->x + container->innerMargin;
        float adjustedY = e->y + container->innerMargin;
        float adjustedWidth  = e->width - 2 * container->innerMargin;
        float adjustedHeight = e->height - 2 * container->innerMargin;

        glPushMatrix();
        glTranslatef(adjustedX, adjustedY, 0.0f);
        {
            e->drawFunc(game, font, adjustedWidth, adjustedHeight);

            /* glColor4f(0.0f, 0.0f, 1.0f, 1.0f); */
            /* glBegin(GL_LINE_LOOP); */
            /* { */
            /*     glVertex2f(0.0f, 0.0f); */
            /*     glVertex2f(adjustedWidth, 0.0f); */
            /*     glVertex2f(adjustedWidth, adjustedHeight); */
            /*     glVertex2f(0.0f, adjustedHeight); */
            /* } */
            /* glEnd(); */
        }
        glPopMatrix();

        /* glPushMatrix(); */
        /* { */
        /*     glColor4f(1.0f, 0.0f, 0.0f, 1.0f); */
        /*     glBegin(GL_LINE_LOOP); */
        /*     { */
        /*         glVertex2f(e->x, e->y); */
        /*         glVertex2f(e->x + e->width, e->y); */
        /*         glVertex2f(e->x + e->width, e->y + e->height); */
        /*         glVertex2f(e->x, e->y + e->height); */
        /*     } */
        /*     glEnd(); */
        /* } */
        /* glPopMatrix(); */
    }
    glPopMatrix();
}
