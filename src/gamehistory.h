#ifndef GAMEHISTORY_H
#define GAMEHISTORY_H

#include "tap_state.h"

#include <utringbuffer.h>
#include <stdbool.h>

#define MAX_GAME_HISTORY_COUNT 32

// Keep a history of ending game states.
struct game_history_t
{
        struct tap_state data[MAX_GAME_HISTORY_COUNT];
        int start;
        int end;
};

void game_history_init(struct game_history_t* gh);
void game_history_terminate(struct game_history_t* gh);

struct game_history_t* game_history_create();
void game_history_destroy(struct game_history_t* gh);

bool isDemoState(UT_ringbuffer* blockHistory);
bool testDemoState(UT_ringbuffer* blockHistory, struct tap_state* demo, size_t demoLength);

void printGameHistory(struct game_history_t* gh);

void pushStateToGameHistory(struct game_history_t* gh,
                            UT_ringbuffer* blockHistory,
                            struct tap_state currentState);
void popGameHistoryElement(struct game_history_t* gh);

// Given an index between start and end, return the associated data element.
struct tap_state* getGameHistoryElement(struct game_history_t* gh, int index);

float averageHistoryStats(struct game_history_t* gh,
                          int (*getVar)(struct tap_state* state));

float averageDeathLevel(struct game_history_t* gh);
float averageMasterGrade(struct game_history_t* gh);

int carnivalScore(struct game_history_t* gh);

#endif /* GAMEHISTORY_H */
