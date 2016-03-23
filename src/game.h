#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdlib.h>

#include <stdint.h>

#include "tap_state.h"

#include <utringbuffer.h>

#define GAME_STATE_HISTORY_LENGTH 32

#define MASTER_S9_INTERNAL_GRADE 31
#define GRADE_COUNT 32

struct input_history_t;
struct section_table_t;
struct game_history_t;

extern const char* DISPLAYED_GRADE[GRADE_COUNT];

const char* getModeName(int gameMode);

struct game_t
{
    int currentSection;

    // We want to detect change on each frame, so we'll keep track of how
    // things looked on the previous frame for comparison.
    struct tap_state curState;
    struct tap_state prevState;

    // Let's also keep a history of locked pieces
    UT_ringbuffer* blockHistory;
};

// (Re)sets all game data. If passed NULL, allocate new game data.
struct game_t* createNewGame(struct game_t* game);
void destroyGame(struct game_t* game, bool freeMem);
void resetGame(struct game_t* game);

bool isInPlayingState(char state);

// Load game state from MAME into our game structure. This also handles adding
// data points to our section data.
void updateGameState(struct game_t* game,
                     struct input_history_t* inputHistory,
                     struct section_table_t* table,
                     struct game_history_t* gh,
                     struct tap_state* dataPtr);

void printGameState(struct game_t* game);

// Will return true if _currently_ not invalidated from getting M-rank.
bool testMasterConditions(struct game_t* game);

/* // Deprecated since we now pull the flag directly from MAME. */
/* bool calculateMasterConditions_(struct game_t* game); */

#endif /* GAME_H */
