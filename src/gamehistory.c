#include "gamehistory.h"

#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

// First Demo: Two simultaneous single player games.
static const size_t demo01_length = 16;
static struct tap_state demo01[] =
{
    { 2, 0, 0, 0, 26, 5, 1, 3, 2, 48, 0, 2 },
    { 2, 0, 0, 1, 75, 8, 4, 3, 2, 48, 0, 2 },
    { 2, 0, 0, 2, 137, 7, 6, 3, 0, 48, 0, 2 },
    { 2, 0, 0, 3, 226, 6, 2, 4, 2, 48, 0, 2 },
    { 2, 0, 0, 4, 291, 4, 2, 5, 0, 48, 0, 2 },
    { 2, 0, 0, 5, 384, 5, 7, 4, 0, 48, 0, 2 },
    { 2, 0, 0, 6, 438, 8, 5, 4, 3, 48, 0, 2 },
    { 2, 0, 0, 7, 491, 3, 0, 5, 1, 48, 0, 2 },
    { 2, 0, 0, 8, 542, 2, 8, 4, 3, 48, 0, 2 },
    { 2, 0, 20, 11, 635, 4, 4, 4, 1, 48, 0, 2 },
    { 2, 0, 20, 12, 692, 7, 6, 4, 0, 48, 0, 2 },
    { 2, 0, 20, 13, 759, 3, 0, 5, 1, 48, 0, 2 },
    { 2, 0, 20, 14, 827, 6, 6, 5, 0, 48, 0, 2 },
    { 2, 0, 19, 15, 883, 2, 7, 4, 1, 48, 0, 2 },
    { 2, 0, 39, 18, 969, 8, 4, 4, 1, 48, 0, 2 },
    { 2, 0, 39, 19, 1035, 5, 2, 3, 1, 48, 0, 2 }
};

// Second Demo: Vs Mode.
static const size_t demo02_length = 14;
static struct tap_state demo02[] =
{
    { 2, 0, 0, 0, 9554, 6, 1, 3, 2, 48, 0, 10 },
    { 2, 0, 0, 1, 9493, 5, 4, 3, 2, 48, 0, 10 },
    { 2, 0, 0, 2, 9444, 2, -1, 5, 3, 48, 0, 10 },
    { 2, 0, 0, 3, 9396, 4, 4, 4, 1, 48, 0, 10 },
    { 2, 0, 0, 4, 9326, 3, 1, 4, 1, 48, 0, 10 },
    { 2, 0, 0, 5, 9265, 8, 1, 6, 1, 48, 0, 10 },
    { 2, 0, 0, 6, 9184, 7, 6, 3, 0, 48, 0, 10 },
    { 2, 0, 0, 7, 9125, 6, 6, 4, 0, 48, 0, 10 },
    { 2, 0, 0, 8, 9075, 5, 8, 3, 1, 48, 0, 10 },
    { 2, 0, 10, 10, 8976, 2, 5, 4, 0, 48, 0, 10 },
    { 2, 0, 10, 11, 8891, 3, 3, 9, 0, 48, 0, 10 },
    { 2, 0, 10, 12, 8805, 7, 5, 9, 0, 48, 0, 10 },
    { 2, 0, 9, 13, 8754, 4, 9, 7, 3, 48, 0, 10 },
    { 2, 0, 29, 16, 8667, 8, 0, 8, 1, 48, 0, 10 }
};

// Third Demo: Doubles Mode.
static const size_t demo03_length = 14;
static struct tap_state demo03[] =
{
    { 2, 0, 0, 0, 32, 5, 1, 3, 2, 48, 0, 4 },
    { 2, 0, 0, 1, 128, 8, 4, 3, 2, 48, 0, 4 },
    { 2, 0, 0, 2, 214, 6, 2, 4, 2, 48, 0, 4 },
    { 2, 0, 0, 3, 270, 7, 0, 5, 0, 48, 0, 4 },
    { 2, 0, 0, 4, 346, 2, 5, 4, 1, 48, 0, 4 },
    { 2, 0, 0, 5, 442, 3, 2, 5, 1, 48, 0, 4 },
    { 2, 0, 0, 6, 512, 5, 1, 6, 2, 48, 0, 4 },
    { 2, 0, 0, 7, 589, 6, 2, 7, 2, 48, 0, 4 },
    { 2, 0, 0, 8, 656, 8, 5, 3, 3, 48, 0, 4 },
    { 2, 0, 20, 11, 754, 4, 2, 6, 0, 48, 0, 4 },
    { 2, 0, 20, 12, 811, 3, 0, 5, 1, 48, 0, 4 },
    { 2, 0, 20, 13, 903, 2, 4, 6, 1, 48, 0, 4 },
    { 2, 0, 19, 14, 989, 7, 2, 7, 0, 48, 0, 4 },
    { 2, 0, 19, 15, 1050, 8, 0, 7, 1, 48, 0, 4 }
};

void game_history_init(struct game_history_t* gh)
{
    gh->start = 0;
    gh->end = 0;
}

void game_history_terminate(struct game_history_t* gh)
{
    (void) gh;
}

struct game_history_t* game_history_create()
{
    struct game_history_t* gh = malloc(sizeof(struct game_history_t));
    if (gh)
    {
        game_history_init(gh);
    }

    return gh;
}

void game_history_destroy(struct game_history_t* gh)
{
    game_history_terminate(gh);
    free(gh);
}

bool isDemoState(UT_ringbuffer* blockHistory)
{
    return
        testDemoState(blockHistory, demo01, demo01_length) ||
        testDemoState(blockHistory, demo02, demo02_length) ||
        testDemoState(blockHistory, demo03, demo03_length);
}

bool testDemoState(UT_ringbuffer* blockHistory, struct tap_state* demo, size_t demoLength)
{
    size_t histLength = utringbuffer_len(blockHistory);

    if (histLength > demoLength)
    {
        return false;
    }

    struct tap_state* current = (struct tap_state*)utringbuffer_front(blockHistory);
    int misses = 0;

    for (size_t i = 0; i < demoLength && current != NULL; ++i)
    {
        if (current->gameMode  != demo[i].gameMode  ||
            current->tetromino != demo[i].tetromino ||
            current->xcoord    != demo[i].xcoord    ||
            current->ycoord    != demo[i].ycoord)
        {
            misses++;

            // Arbitrary number of misses until we flag this sequence as
            // negative
            if (misses >= 6)
            {
                return false;
            }
        }
        else
        {
            current = (struct tap_state*) utringbuffer_next(blockHistory, current);
        }
    }
    return true;
}

void printGameHistory(struct game_history_t* gh)
{
    for (int i = gh->start; i < gh->end; ++i)
    {
        printf("%d %d %d %d %d %d\n",
               gh->data[i].state.gameMode,
               gh->data[i].state.level,
               gh->data[i].state.timer,
               gh->data[i].state.tetromino,
               gh->data[i].state.xcoord,
               gh->data[i].state.ycoord
            );
    }
}

void pushStateToGameHistory(struct game_history_t* gh,
                            UT_ringbuffer* blockHistory,
                            struct tap_state currentState,
                            int gameMode)
{
    if (utringbuffer_empty(blockHistory))
    {
        printf("Empty block history, skipping addition to game history.\n");
        return;
    }

    if (gameMode == TAP_MODE_NULL)
    {
        printf("Null game mode, skipping addition to game history.\n");
        return;
    }

    if (isDemoState(blockHistory))
    {
        printf("Demo state detected, skipping addition to game history.\n");
        return;
    }

    if (gh->end - gh->start >= MAX_GAME_HISTORY_COUNT)
    {
        popGameHistoryElement(gh);
    }

    const size_t elementIndex = (gh->end) % MAX_GAME_HISTORY_COUNT;

    gh->data[elementIndex] = (struct game_history_element_t) { currentState, gameMode };
    gh->end++;
}

void popGameHistoryElement(struct game_history_t* gh)
{
    gh->start++;
}

struct game_history_element_t* getGameHistoryElement(struct game_history_t* gh, int index)
{
    assert(index >= gh->start);
    assert(index <  gh->end);
    return &gh->data[index % MAX_GAME_HISTORY_COUNT];
}

float averageHistoryStats(struct game_history_t* gh, int (*getVar)(struct tap_state* state))
{
    int count = 0;
    int sum= 0;
    for (int i = gh->start; i < gh->end; ++i)
    {
        struct tap_state* state = &getGameHistoryElement(gh, i)->state;

        int var = getVar(state);
        if (var)
        {
            sum += var;
            count++;
        }
    }

    return (float)sum / count;
}

static int getDeathLevelFromState_(struct tap_state* state)
{
    return state->gameMode == TAP_MODE_DEATH ? state->level : 0;
}

static int getMasterGradeFromState_(struct tap_state* state)
{
    return state->gameMode == TAP_MODE_MASTER ? state->grade : 0;
}

float averageDeathLevel(struct game_history_t* gh)
{
    return averageHistoryStats(gh, &getDeathLevelFromState_);
}

float averageMasterGrade(struct game_history_t* gh)
{
    return averageHistoryStats(gh, &getMasterGradeFromState_);
}

int carnivalScore(struct game_history_t* gh)
{
    int sum = 0;
    int count = 0;
    for (int i = gh->end - 1; i >= gh->start; --i)
    {
        struct tap_state* state = &getGameHistoryElement(gh, i)->state;

        if (state->gameMode == TAP_MODE_DEATH)
        {
            sum += state->level;
            count++;

            if (count == 5)
                return sum;
        }
    }

    return sum;
}
