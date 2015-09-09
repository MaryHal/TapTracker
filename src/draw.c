#include "draw.h"
#include "font.h"
#include "game.h"

#include "history.h"
#include "colortheme.h"

#include <stdio.h>

#include <GLFW/glfw3.h>

void setupOpenGL(const unsigned int width, const unsigned int height)
{
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

void drawSectionGraph(struct game_t* game, struct font_t* font,
                      float width, float height)
{
    const float scale = 45.0f;

    const float graphWidth  = width;
    const float graphHeight = height - font->pixelHeight;

    // Gridlines
    setGLColor(COLOR_FOREGROUND, 0.1f);
    for (int i = 0; i < 10; ++i)
    {
        // Vertical lines
        glBegin(GL_LINES);
        {
            glVertex2f(graphWidth * (i + 1) / 10.0f, graphHeight);
            glVertex2f(graphWidth * (i + 1) / 10.0f, 0.0f);
        }
        glEnd();

        // Horizontal lines
        glBegin(GL_LINES);
        {
            glVertex2f(0.0f, graphHeight * i / 10.0f);
            glVertex2f(graphWidth, graphHeight * i / 10.0f);
        }
        glEnd();
    }

    float x = 0.0f;
    float y = 0.0f;

    // Section-Level lines
    struct datapoint_t prevDatapoint = { 0, 0 };
    float sectionAlpha = 0.04f;
    for (int i = game->currentSection - 2; i <= (signed)game->currentSection; ++i)
    {
        if (i < 0)
        {
            sectionAlpha *= 5;
            continue;
        }

        struct section_t* section = &game->sections[i];

        if (section->size == 0)
        {
            continue;
        }

        setGLColor(COLOR_FOREGROUND, sectionAlpha);
        glBegin(GL_LINE_STRIP);
        {
            for (size_t j = 0; j < section->size; ++j)
            {
                struct datapoint_t datapoint = section->data[j];

                x = graphWidth * (convertTime(datapoint.time - section->startTime)) / scale;
                y = graphHeight - graphHeight * (datapoint.level - (i * 100)) / 100.0f;

                int levelDifference = datapoint.level - prevDatapoint.level;
                prevDatapoint = datapoint;

                if (levelDifference > 0)
                {
                    setGLColor(levelDifference - 1, sectionAlpha);
                }
                else
                {
                    setGLColor(COLOR_FOREGROUND, sectionAlpha);
                }
                glVertex2f(x, y);
            }
        }
        glEnd();

        sectionAlpha *= 5;
    }

    // Print level
    char levelStr[15];
    sprintf(levelStr, "%d", prevDatapoint.level);

    setGLColor(COLOR_FOREGROUND, 1.0f);
    drawString(font, x + 4.0f, y - 4.0f, levelStr);

    // Draw axis
    {
        setGLColor(COLOR_FOREGROUND, 1.0f);
        glBegin(GL_LINES);
        {
            glVertex2f(0.0f, 0.0f);
            glVertex2f(0.0f, graphHeight);
        }
        glEnd();

        glBegin(GL_LINES);
        {
            glVertex2f(0.0f, graphHeight);
            glVertex2f(graphWidth, graphHeight);
        }
        glEnd();

        float strWidth = getStringWidth(font, "Section Time");
        drawString(font, (width - strWidth) / 2, graphHeight + font->pixelHeight, "Section Time");

        sprintf(levelStr, "%d", (int)scale);
        drawString(font, width - 10.0f, graphHeight + font->pixelHeight, levelStr);
    }
}

void drawHistory(struct game_t* game, struct font_t* font,
                 float width, float height)
{
    struct history_t* inputHistory = &game->inputHistory;
    float x = 0.0f;
    float y = font->pixelHeight;

    const float vertStride = height / HISTORY_LENGTH;

    for (size_t i = inputHistory->start; i < inputHistory->end; i++)
    {
        setGLColor(COLOR_FOREGROUND, 1.0f);

        char levelString[32];
        sprintf(levelString, "%d:", inputHistory->data[i % HISTORY_LENGTH].level);
        drawString(font, x, y, levelString);

        x = 32.0f;

        struct element_t* element = &inputHistory->data[i % HISTORY_LENGTH];
        for (size_t j = 0; j < element->size; j++)
        {
            if (element->spectrum[j].held)
                setGLColor(COLOR_TETRIS, 1.0f);
            else
                setGLColor(COLOR_FOREGROUND, 1.0f);

            drawChar(font, &x, &y, element->spectrum[j].key);
        }

        x = 0.0f;
        y += vertStride;
    }
}

void drawSectionLineCount(struct game_t* game, struct font_t* font,
                          float width, float height)
{
    float x = 0.0f;
    float y = font->pixelHeight;

    setGLColor(COLOR_FOREGROUND, 1.0f);

    struct section_t* section = &game->sections[game->currentSection];
    char lineCount[64];
    sprintf(lineCount, "%d : %d : %d : %d",
            section->lines[0],
            section->lines[1],
            section->lines[2],
            section->lines[3]);

    float strWidth = getStringWidth(font, lineCount);

    drawString(font, x + (width - strWidth) / 2, y, lineCount);
}
