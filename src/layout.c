#include "layout.h"

#include <stdlib.h>
#include <assert.h>

#include <GLFW/glfw3.h>

void layout_init(struct layout_container_t* c,
                 float width, float height,
                 float outerMargin, float innerMargin)
{
    c->outerMargin = outerMargin;
    c->innerMargin = innerMargin;

    c->leftoverWidth = width - 2 * outerMargin;
    c->leftoverHeight = height - 2 * outerMargin;

    c->size = 0;
}

void layout_terminate(struct layout_container_t* c)
{
    (void) c;
}

struct layout_container_t* layout_create(float width, float height,
                                         float outerMargin, float innerMargin)
{
    struct layout_container_t* c = malloc(sizeof(struct layout_container_t));
    if (c)
    {
        layout_init(c, width, height, outerMargin, innerMargin);
    }

    return c;
}

void layout_destroy(struct layout_container_t* c)
{
    layout_terminate(c);
    free(c);
}

void addToContainerRatio(struct layout_container_t* container, draw_function_p drawFunc,
                         float ratio)
{
    assert(container != NULL);

    addToContainerFixed(container, drawFunc, container->leftoverHeight * ratio);
}

void addToContainerFixed(struct layout_container_t* container, draw_function_p drawFunc,
                         float pixelHeight)
{
    assert(container != NULL && container->size < MAX_LAYOUT_ELEMENTS);

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

void drawLayout(struct layout_container_t* container, bool debug)
{
    float vertices[8];

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
            e->drawFunc(adjustedWidth, adjustedHeight);

            if (debug)
            {
                // Draw inner area
                glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

                vertices[0] = 0.0f;
                vertices[1] = 0.0f;
                vertices[2] = adjustedWidth;
                vertices[3] = 0.0f;
                vertices[4] = adjustedWidth;
                vertices[5] = adjustedHeight;
                vertices[6] = 0.0f;
                vertices[7] = adjustedHeight;

                glEnableClientState(GL_VERTEX_ARRAY);

                glVertexPointer(2, GL_FLOAT, 0, vertices);
                glDrawArrays(GL_LINE_LOOP, 0, 4);

                glDisableClientState(GL_VERTEX_ARRAY);
            }
        }
        glPopMatrix();

        if (debug)
        {
            // Draw outer area
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

            vertices[0] = e->x;
            vertices[1] = e->y;
            vertices[2] = e->x + e->width;
            vertices[3] = e->y;
            vertices[4] = e->x + e->width;
            vertices[5] = e->y + e->height;
            vertices[6] = e->x;
            vertices[7] = e->y + e->height;

            glEnableClientState(GL_VERTEX_ARRAY);

            glVertexPointer(2, GL_FLOAT, 0, vertices);
            glDrawArrays(GL_LINE_LOOP, 0, 4);

            glDisableClientState(GL_VERTEX_ARRAY);
        }
    }
    glPopMatrix();
}
