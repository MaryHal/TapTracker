#include "game.h"

#include <string.h>
#include <assert.h>

bool isInPlayingState(tap_state state)
{
    return state != NONE && state != IDLE && state != STARTUP;
}

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

void pushDataPoint(struct game_t* game, struct datapoint_t datapoint)
{
    const int sectionIndex = datapoint.level / 100;
    assert(sectionIndex >= 0 && sectionIndex < SECTION_COUNT);

    struct section_t* section = &game->sections[sectionIndex];

    // When a datapoint crosses a section boundary, we want that datapoint on
    // both sections.
    if (sectionIndex > 0)
    {
        struct section_t* prevSection = &game->sections[sectionIndex - 1];
        const int levelBoundary = sectionIndex * 100;

        if (prevSection->data[prevSection->size].level < levelBoundary &&
            datapoint.level > levelBoundary)
        {
            pushDataPointToSection(prevSection, datapoint);
        }
    }

    pushDataPointToSection(section, datapoint);
}

void pushDataPointToSection(struct section_t* section, struct datapoint_t datapoint)
{
    // Only push the data point if level has been incremented.
    int levelDifference = 0;
    if (section->size == 0 ||
        (levelDifference = datapoint.level - section->data[section->size - 1].level))
    {
        assert(levelDifference <= 4);

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
