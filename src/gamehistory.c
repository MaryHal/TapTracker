#include "gamehistory.h"

#include <stdlib.h>

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

void pushStateToGameHistory(struct game_history_t* gh, struct tap_state state)
{
    if (gh->end - gh->start == MAX_GAME_HISTORY_COUNT)
    {
        popGameHistoryElement(gh);
    }

    const size_t elementIndex = (gh->end) % MAX_GAME_HISTORY_COUNT;

    gh->data[elementIndex] = state;
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
