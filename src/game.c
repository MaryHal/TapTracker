#include "game.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

float convertTime(int frames)
{
    // Sadly shmupmame runs TAP at 60 fps instead of the native ~61.7 fps.
    return frames / 60.0f;
}

struct game_t* createNewGame(struct game_t* game)
{
    if (game == NULL)
    {
        game = (struct game_t*)malloc(sizeof(struct game_t));
    }

    memset(game, 0, sizeof(struct game_t));

    return game;
}

void destroyGame(struct game_t* game, bool freeMem)
{
    if (freeMem)
        free(game);
}

void resetGame(struct game_t* game)
{
    memset(game, 0, sizeof(struct game_t));
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

void pushDataPoint(struct game_t* game, struct datapoint_t datapoint)
{
    assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT);

    struct section_t* section = &game->sections[game->currentSection];

    // If we're at the end of the game, don't do anything.
    if (section->data[section->size].level >= LEVEL_MAX)
    {
        return;
    }

    const int levelBoundary = (game->currentSection + 1) * SECTION_LENGTH;

    if (datapoint.level >= levelBoundary)
    {
        pushDataPointToSection(section, datapoint);

        // Section advance!
        game->currentSection++;
        section = &game->sections[game->currentSection];
    }

    pushDataPointToSection(section, datapoint);
}

void pushDataPointToSection(struct section_t* section, struct datapoint_t datapoint)
{
    // Only push the data point if level has been incremented.
    int levelDifference = 0;
    if (section->size == 0 ||
        (levelDifference = datapoint.level - section->data[section->size - 1].level) > 0)
    {
        // levelDifference will be zero if this is the first level of the section.
        /* assert(levelDifference >= 0 && levelDifference <= 4); */
        assert(levelDifference >= 0);

        // This section just began, as we have no datapoints yet.
        if (section->size == 0)
        {
            section->startTime = datapoint.time;
        }

        // Push datapoint to the end of the section.
        section->data[section->size] = datapoint;
        section->size++;

        // If we have at least two elements in this section, we can check if we scored some phat lines.
        if (section->size >= 2)
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
