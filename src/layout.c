#include "layout.h"

#include <stdlib.h>
#include <assert.h>

#include <GLFW/glfw3.h>

struct layout_container_t* createLayoutContainer(struct layout_container_t* c,
                                                 float width, float height,
                                                 float outerMargin, float innerMargin)
{
    if (c == NULL)
    {
        c = malloc(sizeof(struct layout_container_t));
    }

    c->outerMargin = outerMargin;
    c->innerMargin = innerMargin;

    c->leftoverWidth = width - 2 * outerMargin;
    c->leftoverHeight = height - 2 * outerMargin;

    c->size = 0;

    return c;
}

void destroyContainer(struct layout_container_t* c, bool freeMe)
{
    if (freeMe)
        free(c);
}

void addToContainerRatio(struct layout_container_t* container, struct draw_container_t drawContainer,
                         float ratio)
{
    assert(container != NULL);

    addToContainerFixed(container, drawContainer, container->leftoverHeight * ratio);
}

void addToContainerFixed(struct layout_container_t* container, struct draw_container_t drawContainer,
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
            .drawContainer = drawContainer
        };

    container->leftoverHeight -= pixelHeight;

    container->size++;
}

void initContainer(struct layout_container_t* container, struct draw_data_t* data_container)
{
    for (size_t i = 0; i < container->size; i++)
    {
        struct layout_element_t* e = &container->elements[i];
        if (e->drawContainer.initFunc != NULL)
        {
            e->drawContainer.initFunc(data_container, e->width, e->height);
        }
    }
}

void drawLayout(struct layout_container_t* container, struct draw_data_t* data_container, bool debug)
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
            e->drawContainer.drawFunc(data_container, adjustedWidth, adjustedHeight);

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
            vertices[0] = e->x + e->width;
            vertices[1] = e->y;
            vertices[0] = e->x + e->width;
            vertices[1] = e->y + e->height;
            vertices[0] = e->x;
            vertices[1] = e->y + e->height;

            glEnableClientState(GL_VERTEX_ARRAY);

            glVertexPointer(2, GL_FLOAT, 0, vertices);
            glDrawArrays(GL_LINE_LOOP, 0, 4);

            glDisableClientState(GL_VERTEX_ARRAY);
        }
    }
    glPopMatrix();
}
