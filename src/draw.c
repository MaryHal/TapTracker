#include "draw.h"
#include "game.h"

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
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);
}

void drawSectionGraph(struct game_t* game, unsigned int x, unsigned int y,
                      unsigned int graphWidth, unsigned int graphHeight)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    {
        // Gridlines
        glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
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

        float colors[4][3] = {
            { 0.8f, 0.8f, 0.8f }, // Single
            { 0.3f, 0.0f, 0.8f }, // Double
            { 0.5f, 0.2f, 0.9f }, // Triple
            { 1.0f, 1.0f, 0.0f }  // Tetris
        };

        // Section-Level lines
        struct datapoint_t prevDatapoint = { 0, 0 };
        float sectionAlpha = 1.0f;
        for (int i = game->currentSection; i >= 0; --i)
        {
            struct section_t* section = &game->sections[i];

            if (section->size == 0)
            {
                continue;
            }

            glColor4f(0.8f, 0.8f, 0.8f, sectionAlpha);
            glBegin(GL_LINE_STRIP);
            {
                for (size_t j = 0; j < section->size; ++j)
                {
                    struct datapoint_t datapoint = section->data[j];

                    float vx = graphWidth * (convertTime(datapoint.time - section->startTime)) / 45.0f;
                    float vy = graphHeight - graphHeight * (datapoint.level - (i * 100)) / 100.0f;

                    int levelDifference = datapoint.level - prevDatapoint.level;
                    prevDatapoint = datapoint;

                    glColor4f(colors[levelDifference - 1][0],
                              colors[levelDifference - 1][1],
                              colors[levelDifference - 1][2],
                              sectionAlpha);

                    glVertex2f(vx, vy);
                }
            }
            glEnd();

            sectionAlpha /= 4;
        }

        // Draw axis
        glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
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
    }
    glPopMatrix();
}
