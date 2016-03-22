#include "gamehistory.h"

#include <stdlib.h>
#include <stdio.h>

// First Demo: Two simultaneous single player games.
static const size_t demo01_length = 17;
static struct tap_state demo01[] =
{
    { 0, 9, 0,  0,   27, 6, 1, 2, 2, 1, 0, 0 },
    { 0, 9, 0,  1,   76, 5, 4, 2, 2, 1, 0, 0 },
    { 0, 9, 0,  2,  138, 3, 6, 3, 0, 1, 0, 0 },
    { 0, 9, 0,  3,  227, 2, 2, 3, 2, 1, 0, 0 },
    { 0, 9, 0,  4,  292, 7, 2, 5, 0, 1, 0, 0 },
    { 0, 9, 0,  5,  385, 6, 7, 4, 0, 1, 0, 0 },
    { 0, 9, 0,  6,  439, 5, 5, 4, 3, 1, 0, 0 },
    { 0, 9, 0,  7,  492, 4, 0, 5, 1, 1, 0, 0 },
    { 0, 9, 0,  8,  544, 1, 9, 4, 3, 1, 0, 0 },
    { 0, 9, 0, 11,  636, 7, 4, 4, 1, 1, 0, 0 },
    { 0, 9, 0, 12,  693, 3, 6, 4, 0, 1, 0, 0 },
    { 0, 9, 0, 13,  760, 4, 0, 5, 1, 1, 0, 0 },
    { 0, 9, 0, 14,  828, 2, 6, 5, 0, 1, 0, 0 },
    { 0, 9, 0, 15,  884, 1, 8, 4, 1, 1, 0, 0 },
    { 0, 9, 0, 18,  970, 5, 4, 4, 1, 1, 0, 0 },
    { 0, 9, 0, 19, 1036, 6, 2, 3, 1, 1, 0, 0 },
    { 0, 9, 0, 19, 1061, 6, 2, 3, 1, 1, 0, 0 },
};

// Second Demo: Vs Mode.
static const size_t demo02_length = 15;
static struct tap_state demo02[] =
{
    { 0, 9, 0,  0, 9553, 2, 1, 2, 2, 1, 0, 0 },
    { 0, 9, 0,  1, 9492, 6, 4, 2, 2, 1, 0, 0 },
    { 0, 9, 0,  2, 9443, 1, 0, 5, 3, 1, 0, 0 },
    { 0, 9, 0,  3, 9395, 7, 4, 4, 1, 1, 0, 0 },
    { 0, 9, 0,  4, 9325, 4, 1, 4, 1, 1, 0, 0 },
    { 0, 9, 0,  5, 9264, 5, 1, 6, 1, 1, 0, 0 },
    { 0, 9, 0,  6, 9183, 3, 6, 3, 0, 1, 0, 0 },
    { 0, 9, 0,  7, 9124, 2, 6, 4, 0, 1, 0, 0 },
    { 0, 9, 0,  8, 9074, 6, 8, 3, 1, 1, 0, 0 },
    { 0, 9, 0, 10, 8975, 1, 5, 4, 0, 1, 0, 0 },
    { 0, 9, 0, 11, 8890, 4, 3, 9, 0, 1, 0, 0 },
    { 0, 9, 0, 12, 8804, 3, 5, 9, 0, 1, 0, 0 },
    { 0, 9, 0, 13, 8753, 7, 9, 7, 3, 1, 0, 0 },
    { 0, 9, 0, 16, 8666, 5, 0, 8, 1, 1, 0, 0 },
    { 0, 9, 0, 17, 8617, 2, 5, 8, 0, 1, 0, 0 },
};

// Third Demo: Doubles Mode.
static const size_t demo03_length = 15;
static struct tap_state demo03[] =
{
    { 0, 9, 0,  0,   33, 6, 1, 2, 2, 1, 0, 0 },
    { 0, 9, 0,  1,  129, 5, 4, 2, 2, 1, 0, 0 },
    { 0, 9, 0,  2,  215, 2, 2, 3, 2, 1, 0, 0 },
    { 0, 9, 0,  3,  271, 3, 0, 5, 0, 1, 0, 0 },
    { 0, 9, 0,  4,  347, 1, 6, 4, 1, 1, 0, 0 },
    { 0, 9, 0,  5,  443, 4, 2, 5, 1, 1, 0, 0 },
    { 0, 9, 0,  6,  513, 6, 1, 5, 2, 1, 0, 0 },
    { 0, 9, 0,  7,  590, 2, 2, 6, 2, 1, 0, 0 },
    { 0, 9, 0,  8,  657, 5, 5, 3, 3, 1, 0, 0 },
    { 0, 9, 0, 11,  755, 7, 2, 6, 0, 1, 0, 0 },
    { 0, 9, 0, 12,  812, 4, 0, 5, 1, 1, 0, 0 },
    { 0, 9, 0, 13,  904, 1, 5, 6, 1, 1, 0, 0 },
    { 0, 9, 0, 14,  990, 3, 2, 7, 0, 1, 0, 0 },
    { 0, 9, 0, 15, 1051, 5, 0, 7, 1, 1, 0, 0 },
    { 0, 9, 0, 15, 1061, 5, 0, 7, 1, 1, 0, 0 },
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

    for (size_t i = 0; i < histLength && i < demoLength; ++i)
    {
        struct tap_state* current = utringbuffer_eltptr(blockHistory, i);

        if (current->tetromino != demo[i].tetromino &&
            current->xcoord    != demo[i].xcoord &&
            current->ycoord    != demo[i].ycoord)
        {
            return false;
        }
    }
    return true;
}

void pushStateToGameHistory(struct game_history_t* gh, UT_ringbuffer* blockHistory)
{
    if (isDemoState(blockHistory))
    {
        printf("Demo state detected, skipping addition to game history.");
        return;
    }

    if (gh->end - gh->start == MAX_GAME_HISTORY_COUNT)
    {
        popGameHistoryElement(gh);
    }

    const size_t elementIndex = (gh->end) % MAX_GAME_HISTORY_COUNT;

    gh->data[elementIndex] = *(struct tap_state*)utringbuffer_back(blockHistory);
    gh->end++;
}

void popGameHistoryElement(struct game_history_t* gh)
{
    gh->start++;
}

float averageHistoryStats(struct game_history_t* gh, int (*getVar)(struct tap_state* state))
{
    int count = 0;
    int sum= 0;
    for (int i = gh->start; i < gh->end; ++i)
    {
        struct tap_state* state = &gh->data[i];

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
