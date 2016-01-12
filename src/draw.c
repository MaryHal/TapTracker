#include "draw.h"
#include "font.h"
#include "game.h"

#include "history.h"
#include "inputhistory.h"

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

void drawSectionGraph(struct draw_data_t* data, float width, float height)
{
    struct game_t* game = data->game;
    struct font_t* font = data->font;

    const float scale = data->scale;

    const float graphWidth  = width;
    const float graphHeight = height - font->pixelHeight;

    // Gridlines (without axis lines)
    setGLColor(COLOR_FOREGROUND, 0.1f);
    float gridlines[4 * 20];
    for (int i = 0; i < 10; ++i)
    {
        // Vertical lines
        gridlines[i * 8 + 0] = graphWidth * (i + 1) / 10.0f;
        gridlines[i * 8 + 1] = graphHeight;
        gridlines[i * 8 + 2] = graphWidth * (i + 1) / 10.0f;
        gridlines[i * 8 + 3] = 0.0f;

        // Horizontal lines
        gridlines[i * 8 + 4] = 0.0f;
        gridlines[i * 8 + 5] = graphHeight * i / 10.0f;
        gridlines[i * 8 + 6] = graphWidth;
        gridlines[i * 8 + 7] = graphHeight * i / 10.0f;
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, gridlines);
    glDrawArrays(GL_LINES, 0, 2 * 20);
    glDisableClientState(GL_VERTEX_ARRAY);

    // Section-Level lines
    float x = 0.0f;
    float y = 0.0f;
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
                    // Since we color our line based on how many levels we
                    // advance, if for some reason we jump a huge amount of
                    // levels, grabbing a color from our color array will result
                    // in an index out of bounds error.
                    if (levelDifference > 4)
                    {
                        levelDifference = 4;
                    }

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

    // Print level at top of section line
    char levelStr[16];
    sprintf(levelStr, "%d", prevDatapoint.level);

    setGLColor(COLOR_FOREGROUND, 1.0f);
    drawString(font, x + 4.0f, y - 4.0f, levelStr);

    // Draw axis
    float axisLines[8] =
        {
            // Vertical
            0.0f, 0.0f,
            0.0f, graphHeight,

            // Horizontal
            0.0f, graphHeight,
            graphWidth, graphHeight
        };

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, axisLines);
    glDrawArrays(GL_LINES, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    // Draw axis labels
    float strWidth = getStringWidth(font, "Section Time");
    drawString(font, (width - strWidth) / 2, graphHeight + font->pixelHeight, "Section Time");

    sprintf(levelStr, "%d", (int)scale);
    drawString(font, width - 10.0f, graphHeight + font->pixelHeight, levelStr);

    // Draw grade and grade points
    if (testMasterConditions(game))
        glColor4f(0.3f, 1.0f, 0.3f, 1.0f);
    else
        glColor4f(1.0f, 0.3f, 0.3f, 1.0f);

    sprintf(levelStr, "%2d %2d",
            game->curState.grade,
            game->curState.gradePoints);
    drawString(font, width - 28.0f, graphHeight - 2.0f, levelStr);

    /* // Draw some block data */
    /* sprintf(levelStr, "%d %d %d %d %d", */
    /*         game->currentBlock, */
    /*         game->nextBlock, */
    /*         game->currentBlockX, */
    /*         game->currentBlockY, */
    /*         game->currentRotState); */
    /* drawString(font, width - 100.0f, graphHeight - 40, levelStr); */
}

void drawInputHistory(struct draw_data_t* data, float width, float height)
{
    (void) width;

    struct history_t* inputHistory = data->history;
    struct button_spectrum_t* bspec = data->bspec;

    float x = 0.0f;
    float y = 0.0f;

    /* const float vertStride = height / HISTORY_LENGTH; */
    const float vertStride = 8.0f;
    const int maxIterations = height / vertStride;

    for (int i = inputHistory->end - maxIterations; i < inputHistory->end; i++)
    {
        if (i < inputHistory->start)
        {
            continue;
        }

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        /* char levelString[32]; */
        /* sprintf(levelString, "%d:", inputHistory->data[i % HISTORY_LENGTH].level); */
        /* drawString(font, x, y, levelString); */

        /* x = 32.0f; */

        struct element_t* element = &inputHistory->data[i % HISTORY_LENGTH];
        for (size_t j = 0; j < element->size; j++)
        {
            unsigned int index = element->spectrum[j].key;
            if (element->spectrum[j].held)
            {
                index += HELD_BUTTON_OFFSET;
            }

            float vertices[8] =
                {
                    x + 0.0f, y + 0.0f,
                    x + vertStride, y + 0.0f,
                    x + vertStride, y + vertStride,
                    x + 0.0f, y + vertStride
                };

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, bspec->textureID);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glVertexPointer(2, GL_FLOAT, 0, vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, bspec->quads[index].texCoords);

            glDrawArrays(GL_QUADS, 0, 4);

            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);

            glDisable(GL_TEXTURE_2D);

            x += vertStride;
        }

        x = 0.0f;
        y += vertStride;
    }
}

void drawSectionTable(struct draw_data_t* data, float width, float height)
{
    (void) width;

    struct game_t* game = data->game;
    struct font_t* font = data->font;

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
        if (game->curState.level >= (i + 1) * SECTION_LENGTH || game->curState.level >= LEVEL_MAX)
        {
            sectionTime = frameTimeToSeconds(section->endTime - section->startTime);
        }
        else
        {
            sectionTime = frameTimeToSeconds(game->curState.timer - section->startTime);
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
