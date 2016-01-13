#include "game.h"
#include "sectiontime.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

const char* DISPLAYED_GRADE[GRADE_COUNT] =
{
    "9", "8", "7", "6", "5", "4-", "4+", "3-", "3+", "2-", "2", "2+", "1-",
    "1", "1+", "S1-", "S1", "S1+", "S2", "S3", "S4-", "S4", "S4+", "S5-",
    "S5+", "S6-", "S6+", "S7-", "S7+", "S8-", "S8+", "S9"
};

float frameTimeToSeconds(int frames)
{
    return frames / TIMER_FPS;
}

int frameTime(float seconds)
{
    return (int)(seconds * TIMER_FPS);
}

int getSectionTime(struct section_t* section)
{
    return section->endTime - section->startTime;
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
}

void resetGame(struct game_t* game)
{
    memset(game, 0, sizeof(struct game_t));

    // Push an initial (blank) data point from the game's initial state.
    pushCurrentState(game, NULL);
}

bool isGameComplete(struct game_t* game)
{
    struct section_t* section = getSection(game, game->currentSection);
    return section->data[section->size].level >= LEVEL_MAX;
}

bool isInPlayingState(char state)
{
    return state != TAP_NONE && state != TAP_IDLE && state != TAP_STARTUP;
}

void updateGameState(struct game_t* game, struct history_t* inputHistory, struct section_table_t* table,
                     struct tap_state* dataPtr)
{
    game->prevState = game->curState;

    // Copy the data that we set in the MAME process.
    game->curState = *dataPtr;

    if (isInPlayingState(game->curState.state) && game->curState.level < game->prevState.level)
    {
        perror("Internal State Error");
        printGameState(game);
    }

    if (isInPlayingState(game->curState.state) && game->curState.level - game->prevState.level > 0)
    {
        // Push a data point based on the newly acquired game state.
        pushCurrentState(game, table);
    }

    if (game->prevState.state != TAP_ACTIVE && game->curState.state == TAP_ACTIVE)
    {
        pushHistoryElement(inputHistory, game->curState.level);
    }

    // Check if a game has ended
    if (isInPlayingState(game->prevState.state) && !isInPlayingState(game->curState.state))
    {
        updateSectionTime(table, game->prevState.gameMode);

        resetGame(game);
        resetHistory(inputHistory);
    }
}

void pushCurrentState(struct game_t* game, struct section_table_t* table)
{
    assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT);

    struct section_t* section = getSection(game, game->currentSection);

    // Special case for when we're at the end of the game (level 999). The
    // in-game timer seems to reset back to 00:00:00 on the same exact frame
    // that level 999 is reached. So when we're adding our data point, we must
    // use the previous frame's timer value.
    if (game->curState.level >= LEVEL_MAX)
    {
        if (section->endTime == 0)
        {
            section->endTime = game->prevState.timer;

            int tempTime = game->curState.timer;
            game->curState.timer = game->prevState.timer;

            addDataPointToSection(game, section);

            // Log section time
            setSectionTableTime(table, game->currentSection, section->endTime - section->startTime);

            // For consistency, restore original time value.
            game->curState.timer = tempTime;
        }

        return;
    }

    // If we've passed this section's boundary, add our last point and move to
    // the next section.
    while (game->curState.level >= (game->currentSection + 1) * SECTION_LENGTH)
    {
        section->endTime = game->curState.timer;
        addDataPointToSection(game, section);

        // Log section time
        setSectionTableTime(table, game->currentSection, section->endTime - section->startTime);

        // Section advance!
        game->currentSection++;
        section = getSection(game, game->currentSection);

        assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT);
    }

    addDataPointToSection(game, section);
}

void addDataPointToSection(struct game_t* game, struct section_t* section)
{
    assert(section->size < SECTION_MAX - 1);

    // Section just began
    if (section->size == 0)
    {
        section->startTime = game->curState.timer;

        // Push datapoint to the end of the section.
        section->data[section->size] = (struct datapoint_t) { game->curState.level, game->curState.timer };
        section->size++;
    }
    else
    {
        // Only push the data point if level has been incremented.
        int levelDifference = levelDifference = game->curState.level - section->data[section->size - 1].level;

        if (levelDifference > 0)
        {
            if (levelDifference > 4)
            {
                fprintf(stderr,
                        "Abnormally large level jump: %d -> %d\n",
                        section->data[section->size - 1].level,
                        game->curState.level);
            }

            // Push datapoint to the end of the section.
            section->data[section->size] = (struct datapoint_t) { game->curState.level, game->curState.timer };
            section->size++;

            // Check if we scored some phat lines.
            if (section->size >= 2 &&
                (game->prevState.state == TAP_LOCKING || game->curState.state == TAP_LINECLEAR))
            {
                section->lines[levelDifference - 1]++;
            }
        }
    }

    assert(section->size <= SECTION_MAX);
}

struct section_t* getSection(struct game_t* game, int sectionIndex)
{
    assert(sectionIndex < SECTION_COUNT);
    return &game->sections[sectionIndex];
}

bool testMasterConditions(struct game_t* game)
{
    return
        game->curState.mrollFlags == M_NEUTRAL ||
        game->curState.mrollFlags == M_PASS_1  ||
        game->curState.mrollFlags == M_PASS_2  ||
        game->curState.mrollFlags == M_SUCCESS;
}

bool calculateMasterConditions_(struct game_t* game)
{
    int sectionSum = 0;
    const int TETRIS_INDEX = 3;

    for (int i = 0; i < game->currentSection; i++)
    {
        struct section_t* section = &game->sections[i];

        // First 5 sections must be completed in 1:05:00 or less
        if (i < 5)
        {
            if (getSectionTime(section) > frameTime(65))
            {
                return false;
            }
            sectionSum += getSectionTime(section);

            // Two tetrises per section is required for the first 5 sections.
            if (section->lines[TETRIS_INDEX] < 2)
            {
                return false;
            }
        }
        // Sixth section (500-600) must be less than two seconds slower than the
        // average of the first 5 sections.
        else if (i == 5)
        {
            if (getSectionTime(section) > frameTime(sectionSum / 5 + 2))
            {
                return false;
            }

            // One tetris is required for the sixth section.
            if (section->lines[TETRIS_INDEX] < 1)
            {
                return false;
            }
        }
        // Last four sections must be less than two seconds slower than the
        // previous section.
        else
        {
            struct section_t* prevSection = &game->sections[i - 1];

            if (getSectionTime(section) > getSectionTime(prevSection) + frameTime(2))
            {
                return false;
            }

            // One tetris is required for the last four sections EXCEPT the last
            // one.
            if (section->lines[TETRIS_INDEX] < 1)
            {
                return false;
            }
        }
    }

    // Finally, an S9 grade is required at level 999 along with the same time
    // requirements as the eigth section.
    if (game->curState.level == LEVEL_MAX)
    {
        if (game->curState.grade < MASTER_S9_INTERNAL_GRADE)
        {
            return false;
        }

        // Hard time requirement over the entire game is 8:45:00
        if (game->sections[9].endTime - game->sections[0].startTime > frameTime(8 * 60 + 45))
        {
            return false;
        }

        // Test section time vs previous section
        struct section_t* section = &game->sections[9];
        struct section_t* prevSection = &game->sections[8];

        if (getSectionTime(section) > getSectionTime(prevSection) + frameTime(2))
        {
            return false;
        }
    }

    return true;
}

void printGameState(struct game_t* game)
{
    printf("state: %d -> %d, level %d -> %d, time %d -> %d\n",
           game->prevState.state, game->curState.state,
           game->prevState.level, game->curState.level,
           game->prevState.timer, game->curState.timer);
}
