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
                      float width, float height,
                      void* param)
{
    const float scale = *(float*)param;

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

                x = graphWidth * (frameTimeToSeconds(datapoint.time - section->startTime)) / scale;
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
    char levelStr[32];
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

        // Draw grade and grade points
        if (game->masterQualified)
            glColor4f(0.3f, 1.0f, 0.3f, 1.0f);
        else
            glColor4f(1.0f, 0.3f, 0.3f, 1.0f);

        sprintf(levelStr, "%2d %2d %2d %2d %2d",
                game->MrollFlags,
                game->inCreditRoll,
                game->sectionIndex,
                game->grade,
                game->gradePoints);
        /* drawString(font, width - 28.0f, graphHeight - 2.0f, levelStr); */
        drawString(font, width - 80.0f, graphHeight - 2.0f, levelStr);
    }
}

void drawInputHistory(struct game_t* game, struct font_t* font,
                      float width, float height,
                      void* param)
{
    struct history_t* inputHistory = &game->inputHistory;
    float x = 0.0f;
    float y = font->pixelHeight;

    /* const float vertStride = height / HISTORY_LENGTH; */
    const float vertStride = font->pixelHeight;
    const int maxIterations = height / vertStride;

    /* for (int i = inputHistory->end - 1; inputHistory->end - i <= maxIterations && i >= inputHistory->start; i--) */
    for (int i = inputHistory->end - maxIterations; i < inputHistory->end; i++)
    {
        if (i < inputHistory->start)
        {
            continue;
        }

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

void drawSectionTable(struct game_t* game, struct font_t* font,
                      float width, float height,
                      void* param)
{
    const float vertStride = font->pixelHeight;
    const int maxIterations = height / vertStride;

    float y = vertStride;

    setGLColor(COLOR_FOREGROUND, 1.0f);

    for (int i = game->currentSection - maxIterations; i <= (signed)game->currentSection; ++i)
    {
        if (i < 0)
        {
            continue;
        }

        struct section_t* section = &game->sections[i];

        // Calculate how long this section took / is taking.
        float sectionTime = 0.0f;
        if (game->level >= (i + 1) * SECTION_LENGTH || game->level == LEVEL_MAX)
        {
            sectionTime = frameTimeToSeconds(section->endTime - section->startTime);
        }
        else
        {
            sectionTime = frameTimeToSeconds(game->time - section->startTime);
        }

        char sectionString[16];
        sprintf(sectionString, "%03d-%03d:", i * SECTION_LENGTH, (i + 1) * SECTION_LENGTH - 1);

        char timeString[32];
        sprintf(timeString, "%.2f", sectionTime);

        char lineCount[20];
        sprintf(lineCount, "%2d : %2d : %2d : %2d",
                section->lines[0],
                section->lines[1],
                section->lines[2],
                section->lines[3]);

        drawString(font,   0.0f, y, sectionString);
        drawString(font,  58.0f, y, timeString);
        drawString(font, 106.0f, y, lineCount);

        y += vertStride;
    }
}
