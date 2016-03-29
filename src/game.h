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

bool isVersusMode(int gameMode);
bool is20GMode(int gameMode);
bool isBigMode(int gameMode);
bool isItemMode(int gameMode);
bool isTLSMode(int gameMode);

int getBaseMode(int gameMode);

void getModeName(char* buffer, size_t bufferLength, int gameMode);

struct game_t
{
    int currentSection;

    // When the game is completed, tgm2p throws away all mode modifiers (20G,
    // Versus, Item), so let's keep track of the game mode that we started the
    // game with.
    int originalGameMode;

    // We want to detect change on each frame, so we'll keep track of how
    // things looked on the previous frame for comparison.
    struct tap_state curState;
    struct tap_state prevState;

    // Let's also keep a history of locked pieces
    UT_ringbuffer* blockHistory;
};

void game_init(struct game_t* g);
void game_terminate(struct game_t* g);

struct game_t* game_create();
void game_destroy(struct game_t* g);

// (Re)sets all game data. If passed NULL, allocate new game data.
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
