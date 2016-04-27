#include "draw.h"

#include "game.h"
#include "font.h"

#include "util.h"

#include "inputhistory.h"
#include "buttonquads.h"

#include "sectiontable.h"
#include "gamehistory.h"

#include "colortheme.h"

#include <stdio.h>

#include <assert.h>
#include <zf_log.h>

#include <GLFW/glfw3.h>

static struct draw_data_t data = {0};

void bindDrawData(struct draw_data_t d)
{
    data.game = d.game;
    data.font = d.font;

    data.history = d.history;
    data.bspec = d.bspec;

    data.table = d.table;
    data.gh = d.gh;
}

draw_function_p stringToDrawFunc(const char* functionIdentifier)
{
    assert(functionIdentifier != NULL);

    if (strcmp(functionIdentifier, "section-graph") == 0)
        return &drawSectionGraph;
    else if (strcmp(functionIdentifier, "input-history") == 0)
        return &drawInputHistory;
    else if (strcmp(functionIdentifier, "line-count") == 0)
        return &drawLineCount;
    else if (strcmp(functionIdentifier, "current-state") == 0)
        return &drawCurrentState;
    else if (strcmp(functionIdentifier, "section-table-old") == 0)
        return &drawSectionTable;
    else if (strcmp(functionIdentifier, "section-table") == 0)
        return &drawSectionTableOverall;
    else if (strcmp(functionIdentifier, "game-history") == 0)
        return &drawGameHistory;
    else if (strcmp(functionIdentifier, "mroll-test") == 0)
        return &drawMRollPassFail;

    return NULL;
}

bool drawSectionGraph(float width, float height)
{
    struct game_t* game = data.game;
    struct font_t* font = data.font;

    struct section_table_t* table = data.table;

    if (!table)
    {
        ZF_LOGW("drawSectionGraph: section_table_t is NULL.");
        return false;
    }

    float scale = 0.0f;

    if (getBaseMode(game->curState.gameMode) == TAP_MODE_DEATH)
        scale = 45.0f;
    else
        scale = 60.0f;

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

        struct section_t* section = &table->sections[i];

        if (section->size == 0)
        {
            continue;
        }

        float graphVertices[2 * section->size];
        float graphColors[4 * section->size];

        setGLColor(COLOR_FOREGROUND, sectionAlpha);
        for (size_t j = 0; j < section->size; ++j)
        {
            struct datapoint_t datapoint = section->data[j];

            x = graphWidth * (frameTimeToSeconds(datapoint.time - section->startTime)) / scale;
            y = graphHeight - graphHeight * (datapoint.level - (i * 100)) / 100.0f;

            graphVertices[j * 2 + 0] = x;
            graphVertices[j * 2 + 1] = y;

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

                const float* color = getColorFromTheme(levelDifference - 1);
                graphColors[j * 4 + 0] = color[0];
                graphColors[j * 4 + 1] = color[1];
                graphColors[j * 4 + 2] = color[2];
                graphColors[j * 4 + 3] = sectionAlpha;
            }
            else
            {
                const float* color = getColorFromTheme(COLOR_FOREGROUND);
                graphColors[j * 4 + 0] = color[0];
                graphColors[j * 4 + 1] = color[1];
                graphColors[j * 4 + 2] = color[2];
                graphColors[j * 4 + 3] = sectionAlpha;
            }
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, graphVertices);
        glColorPointer(4, GL_FLOAT, 0, graphColors);
        glDrawArrays(GL_LINE_STRIP, 0, section->size);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

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
    drawString(font, width - 12.0f, graphHeight + font->pixelHeight, levelStr);

    // Draw grade and grade points
    if (testMasterConditions(&game->curState))
        glColor4f(0.3f, 1.0f, 0.3f, 1.0f);
    else
        glColor4f(1.0f, 0.3f, 0.3f, 1.0f);

    sprintf(levelStr, "%2d %2d",
            game->curState.grade,
            game->curState.gradePoints);
    drawString(font, width - 30.0f, graphHeight - 2.0f, levelStr);

    /* // Draw some block data */
    /* sprintf(levelStr, "%d %d %d %d %d", */
    /*         game->currentBlock, */
    /*         game->nextBlock, */
    /*         game->currentBlockX, */
    /*         game->currentBlockY, */
    /*         game->currentRotState); */
    /* drawString(font, width - 100.0f, graphHeight - 40, levelStr); */

    return true;
}

bool drawInputHistory(float width, float height)
{
    (void) width;

    struct input_history_t* inputHistory = data.history;
    struct button_spectrum_t* bspec = data.bspec;

    if (!inputHistory)
    {
        ZF_LOGE("drawInputHistory: input_history_t is NULL");
        return false;
    }

    float x = 0.0f;
    float y = 0.0f;

    const int tileSize = 12;
    const int margin = 2;
    const int vertStride = tileSize + margin;
    const int maxIterations = height / vertStride;

    for (int i = inputHistory->end - maxIterations; i < inputHistory->end; i++)
    {
        if (i < inputHistory->start)
        {
            continue;
        }

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        struct element_t* element = getInputHistoryElement(inputHistory, i);
        for (size_t j = 0; j < element->size; j++)
        {
            unsigned int index = element->spectrum[j].jkey;
            if (element->spectrum[j].held)
            {
                index += HELD_BUTTON_OFFSET;
            }

            float vertices[8] =
                {
                    x + 0.0f, y + 0.0f,
                    x + tileSize, y + 0.0f,
                    x + tileSize, y + tileSize,
                    x + 0.0f, y + tileSize
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

            x += tileSize;
        }

        x = 0.0f;
        y += vertStride;
    }

    return true;
}

bool drawLineCount(float width, float height)
{
    (void) height;

    struct game_t* game = data.game;
    struct font_t* font = data.font;
    struct section_table_t* table = data.table;

    struct section_t* section = &table->sections[game->currentSection];

    char lineCount[20];
    snprintf(lineCount, 20, "%2d : %2d : %2d : %2d",
            section->lines[0],
            section->lines[1],
            section->lines[2],
            section->lines[3]);

    setGLColor(COLOR_FOREGROUND, 1.0f);

    drawString(font, (width - getStringWidth(font, lineCount)) / 2, font->pixelHeight, lineCount);

    return true;
}

bool drawCurrentState(float width, float height)
{
    (void) width, (void) height;

    struct game_t* game = data.game;
    struct font_t* font = data.font;
    struct section_table_t* table = data.table;

    struct section_t* section = &table->sections[game->currentSection];

    char timerStr[16];
    formatTimeToMinutes(timerStr, 16, game->curState.timer);

    char curSectionStr[16];
    formatTimeToSeconds(curSectionStr, 16, game->curState.timer - section->startTime);

    char gameStatsStr[64];
    snprintf(gameStatsStr, 64,
             "lvl %03d     %s         %s",
             game->curState.level,
             timerStr,
             curSectionStr);

    /* char lineCount[64]; */
    /* snprintf(lineCount, 64, "Sec. Lines %2d : %2d : %2d : %2d", */
    /*         section->lines[0], */
    /*         section->lines[1], */
    /*         section->lines[2], */
    /*         section->lines[3]); */

    setGLColor(COLOR_FOREGROUND, 1.0f);

    drawString(font, 0.0f, font->pixelHeight, gameStatsStr);
    /* drawString(font, 0.0f, font->pixelHeight * 2, lineCount); */

    return true;
}

bool drawSectionTable(float width, float height)
{
    (void) width;

    struct game_t* game = data.game;
    struct font_t* font = data.font;
    struct section_table_t* table = data.table;

    const float vertStride = font->pixelHeight;
    const int maxIterations = height / vertStride;

    float y = vertStride;

    setGLColor(COLOR_FOREGROUND, 1.0f);

    for (int i = game->currentSection - maxIterations; i <= game->currentSection; ++i)
    {
        if (i < 0)
        {
            continue;
        }

        struct section_t* section = &table->sections[i];

        drawString(font, 0.0f, y, section->label);

        // Calculate how long this section took / is taking.
        float sectionTime = 0.0f;

        if (section->complete)
        {
            int sectionTimeInFrames = table->sections[i].endTime - table->sections[i].startTime;
            sectionTime = frameTimeToSeconds(sectionTimeInFrames);

            struct pb_table_t* pb = addPBTable(&table->pbHash, game->originalGameMode);

            if (pb->goldST[i] < sectionTimeInFrames)
            {
                setGLColor(COLOR_RED, 1.0f);
            }
            else
            {
                setGLColor(COLOR_GREEN, 1.0f);
            }
        }
        else // Section is still in progress
        {
            sectionTime = frameTimeToSeconds(game->curState.timer - section->startTime);
        }

        char timeString[32];
        sprintf(timeString, "%.2f", sectionTime);

        drawString(font,  58.0f, y, timeString);

        char lineCount[20];
        sprintf(lineCount, "%2d : %2d : %2d : %2d",
                section->lines[LINE_SINGLE],
                section->lines[LINE_DOUBLE],
                section->lines[LINE_TRIPLE],
                section->lines[LINE_TETRIS]);

        setGLColor(COLOR_FOREGROUND, 1.0f);
        drawString(font, 106.0f, y, lineCount);

        y += vertStride;
    }

    return true;
}

bool drawSectionTableOverall(float width, float height)
{
    (void) width, (void) height;

    struct game_t* game = data.game;
    struct font_t* font = data.font;
    struct section_table_t* table = data.table;

    const float vertStride = font->pixelHeight;
    const int maxIterations = height / vertStride + 1;

    float y = vertStride;

    // Make sure the current section is visible and, preferably, centered (if
    // all sections cannot be seen).
    int top = game->currentSection - maxIterations / 2;
    int bottom = top + maxIterations;

    if (top < 0)
    {
        bottom += -top;
        top = 0;
    }

    const int NUM_SECTIONS = getModeSectionCount(game->originalGameMode);
    if (bottom > NUM_SECTIONS)
    {
        top -= bottom - NUM_SECTIONS;
        bottom = NUM_SECTIONS;

        if (top < 0)
            top = 0;
    }

    for (int i = top; i < bottom; ++i)
    {
        struct section_t* section = &table->sections[i];
        struct pb_table_t* pb = addPBTable(&table->pbHash, game->originalGameMode);

        int overallPB = pb->gameTime[i];
        int sectionPB = pb->goldST[i];

        if (!section->complete)
        {
            setGLColor(COLOR_FOREGROUND, 0.3f);

            drawString(font, 0.0f, y, section->label);

            char sectionTime[16];
            formatTimeToMinutes(sectionTime, 16, overallPB);

            drawString(font, 71.0f, y, sectionTime);

            char sectionTimeDiff[16];
            formatTimeToSeconds(sectionTimeDiff, 16, sectionPB);

            drawString(font, 172.0f, y, sectionTimeDiff);

            // Tetris count
            setGLColor(COLOR_FOREGROUND, 0.2f);

            char tetrisCount[6];
            snprintf(tetrisCount, 6, "%d*", section->lines[LINE_TETRIS]);

            drawString(font, 50.0f, y, tetrisCount);

        }
        else
        {
            setGLColor(COLOR_FOREGROUND, 1.0f);

            drawString(font, 0.0f, y, section->label);

            if (section->mTest)
            {
                setGLColor(COLOR_TETRIS, 1.0f);
            }
            else
            {
                setGLColor(COLOR_FOREGROUND, 1.0f);
            }

            char tetrisCount[6];
            snprintf(tetrisCount, 6, "%d*", section->lines[LINE_TETRIS]);

            drawString(font, 50.0f, y, tetrisCount);

            char sectionTime[16];
            formatTimeToMinutes(sectionTime, 16, section->endTime);

            setGLColor(COLOR_FOREGROUND, 1.0f);
            drawString(font, 71.0f, y, sectionTime);

            char overallTimeDiff[16];
            formatTimeToSeconds(overallTimeDiff, 16, section->endTime - overallPB);

            // Set color so it's like all the speedrun timers.
            if (overallPB < section->endTime)
                setGLColor(COLOR_RED, 1.0f);
            else
                setGLColor(COLOR_GREEN, 1.0f);

            drawString(font, 124.0f, y, overallTimeDiff);

            char sectionTimeDiff[16];
            /* formatTimeToSeconds(sectionTimeDiff, 16, section->endTime - section->startTime - sectionPB); */
            formatTimeToSeconds(sectionTimeDiff, 16, section->endTime - section->startTime);

            // Set color so it's like all the speedrun timers.
            if (sectionPB > section->endTime - section->startTime)
                setGLColor(COLOR_TETRIS, 1.0f);
            else
                setGLColor(COLOR_FOREGROUND, 1.0f);

            drawString(font, 172.0f, y, sectionTimeDiff);
        }

        y += vertStride;
    }

    return true;
}

bool drawGameHistory(float width, float height)
{
    (void) width;

    /* struct game_t* game = data.game; */
    struct font_t* font = data.font;
    struct game_history_t* gh = data.gh;

    const float vertStride = font->pixelHeight;
    const int maxIterations = height / vertStride - 2;

    float x = 0.0f;
    float y = vertStride;

    setGLColor(COLOR_FOREGROUND, 1.0f);

    for (int i = gh->end - maxIterations; i < gh->end; i++)
    {
        if (i < gh->start)
        {
            continue;
        }

        struct game_history_element_t* gh_element = getGameHistoryElement(gh, i);

        char gameTimeStr[16];
        formatTimeToMinutes(gameTimeStr, 16, gh_element->state.timer);

        char modeName[32];
        getModeName(modeName, 32, gh_element->gameMode);

        char buf[64];
        snprintf(buf, 32, "%s %03d @ %s",
                 modeName,
                 gh_element->state.level,
                 gameTimeStr);

        drawString(font, x, y, buf);

        x = 0.0f;
        y += vertStride;
    }

    char buf[32];
    snprintf(buf, 32, "Death Carnival Score: %d", carnivalScore(gh));

    drawString(font, x, y, buf);

    return true;
}

bool drawMRollPassFail(float width, float height)
{
    struct game_t* game = data.game;

    if (testMasterConditions(&game->curState))
        glColor4f(0.3f, 1.0f, 0.3f, 1.0f);
    else
        glColor4f(1.0f, 0.3f, 0.3f, 1.0f);

    float box[2 * 4];

    box[0] = 0;
    box[1] = 0;
    box[2] = width;
    box[3] = 0;
    box[4] = 0;
    box[5] = height;
    box[6] = width;
    box[7] = height;

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, box);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    return true;
}
