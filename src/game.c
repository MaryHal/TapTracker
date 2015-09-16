#include "game.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

float convertTime(int frames)
{
    return frames / 60.0f;
}

struct game_t* createNewGame(struct game_t* game)
{
    if (game == NULL)
    {
        game = (struct game_t*)malloc(sizeof(struct game_t));
    }

    resetGame(game);

    return game;
}

void destroyGame(struct game_t* game, bool freeMem)
{
    if (freeMem)
        free(game);

    destroyHistory(&game->inputHistory, false);
}

void resetGame(struct game_t* game)
{
    memset(game, 0, sizeof(struct game_t));

    resetHistory(&game->inputHistory);

    // Push an initial data point from the game's initial state.
    pushCurrentState(game);
}

bool isGameComplete(struct game_t* game)
{
    struct section_t* section = &game->sections[game->currentSection];
    return section->data[section->size].level >= LEVEL_MAX;
}

bool isInPlayingState(tap_state state)
{
    return state != NONE && state != IDLE && state != STARTUP;
}

void pushCurrentState(struct game_t* game)
{
    assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT);

    struct section_t* section = &game->sections[game->currentSection];

    // Test if we're still qualified!
    game->masterQualified = testMasterConditions(game);

    // If we're at the end of the game, don't do anything.
    if (section->data[section->size].level >= LEVEL_MAX)
    {
        return;
    }

    const int levelBoundary = (game->currentSection + 1) * SECTION_LENGTH;

    // TODO: Figure out what causes the input (from MAME) inconsistency that
    // distorts the game state for a single frame. To patch this, we will use
    // the previous frame's state if a problem occurs. The main will still
    // output an error during these inconsistent frames.
    int currentLevel = (game->level < game->prevLevel) ? game->prevLevel : game->level;
    int currentTime  = (game->time < game->prevTime) ? game->prevTime : game->time;

    if (currentLevel >= levelBoundary)
    {
        section->endTime = currentTime;
        addDataPointToSection(game, section, currentLevel, currentTime);

        // Section advance!
        game->currentSection++;
        section = &game->sections[game->currentSection];
    }

    addDataPointToSection(game, section, currentLevel, currentTime);
}

void addDataPointToSection(struct game_t* game, struct section_t* section,
                           int currentLevel, int currentTime)
{
    // Only push the data point if level has been incremented.
    int levelDifference = 0;
    if (section->size == 0 ||
        (levelDifference = currentLevel - section->data[section->size - 1].level) > 0)
    {
        // levelDifference will be zero if this is the first level of the section.
        /* assert(levelDifference >= 0 && levelDifference <= 4); */
        assert(levelDifference >= 0);

        // This section just began, as we have no datapoints yet.
        if (section->size == 0)
        {
            section->startTime = currentTime;
        }

        // Push datapoint to the end of the section.
        section->data[section->size] = (struct datapoint_t) { currentLevel, currentTime };
        section->size++;

        // If we have at least two elements in this section, we can check if we
        // scored some phat lines. Sometimes the state on line clear is not set
        // at the correct time (it stays in the LOCKING state).
        /* if (game->state == LOCKING) */
        /* { */
        /*     perror("Line clear is on LOCKING state instead of LINECLEAR state!\n"); */
        /* } */
        /* if (section->size >= 2 && (game->state == LINECLEAR || game->state == LOCKING)) */
        if (section->size >= 2 && (game->prevState == LOCKING || game->state == LINECLEAR))
        {
            section->lines[levelDifference - 1]++;
        }
    }

    assert(section->size <= SECTION_MAX);
}

struct section_t* getSection(struct game_t* game, int sectionIndex)
{
    assert(sectionIndex < SECTION_COUNT);
    return &game->sections[sectionIndex];
}

void printGameState(struct game_t* game)
{
    printf("state: %d -> %d, level %d -> %d, time %d -> %d\n",
           game->prevState, game->state,
           game->prevLevel, game->level,
           game->prevTime, game->time);
}

bool testMasterConditions(struct game_t* game)
{
    int sectionSum = 0;

    for (unsigned int i = 0; i < game->currentSection; i++)
    {
        struct section_t* section = &game->sections[i];
        // First 5 sections must be completed in 1:05:00 or less
        if (i < 5)
        {
            if (section->endTime - section->startTime > 65 * 60)
            {
                return false;
            }
            sectionSum += section->endTime - section->startTime;

            // Two tetrises per section is required for the first 5 sections.
            if (section->lines[3] < 2)
            {
                return false;
            }
        }
        // Sixth section (500-600) must be less than two seconds slower than the
        // average of the first 5 sections.
        else if (i == 5)
        {
            if (section->endTime - section->startTime > sectionSum / 5 + 2 * 60)
            {
                return false;
            }

            // One tetris is required for the sixth section.
            if (section->lines[3] < 1)
            {
                return false;
            }
        }
        // Last four sections must be less than two seconds slower than the
        // previous section.
        else
        {
            struct section_t* prevSection = &game->sections[i - 1];
            if (section->endTime - section->startTime > prevSection->endTime - prevSection->startTime + 2 * 60)
            {
                return false;
            }

            // One tetris is required for the last four sections EXCEPT the last
            // one.
            if (section->lines[3] < 1)
            {
                return false;
            }
        }
    }

    // Finally, an S9 grade is required at level 999 along with the same time
    // requirements as the eigth section.
    if (game->level == 999)
    {
        if (game->grade < 31)
        {
            return false;
        }

        // Test section time vs previous section
        struct section_t* section = &game->sections[8];
        struct section_t* prevSection = &game->sections[9];
        if (section->endTime - section->startTime > prevSection->endTime - prevSection->startTime + 2 * 60)
        {
            return false;
        }
    }

    return true;
}
