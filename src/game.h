#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdlib.h>

#include "history.h"

#define LEVEL_MAX      999

#define SECTION_LENGTH 100
#define SECTION_MAX    100
#define SECTION_COUNT  10

typedef enum
{
    NONE         = 0,
    /* PRE_ENTRY   = 1, */
    ACTIVE       = 2,
    LOCKING      = 3, // Cannot be influenced anymore
    LINECLEAR    = 4, // Tetromino is being locked to the playfield.
    ENTRY        = 5,
    GAMEOVER     = 7,  // "Game Over" is being shown on screen.
    IDLE         = 10, // No game has started, just waiting...
    FADING       = 11, // Blocks fading away when topping out (losing).
    COMPLETION   = 13, // Blocks fading when completing the game
    STARTUP      = 71
} tap_state;

struct datapoint_t
{
        int level;
        int time;
};

struct section_t
{
        struct datapoint_t data[SECTION_MAX];
        size_t size;

        int startTime; // Frame count for when this section began.
        int lines[4];
};

struct game_t
{
        struct section_t sections[SECTION_COUNT];
        unsigned int currentSection;

        // We want to detect change on each frame, so we'll keep track of how
        // things looked on the previous frame for comparison.
        tap_state state, prevState;
        int level, prevLevel;
        int time, prevTime;

        struct history_t inputHistory;
};

float convertTime(int frames);

// (Re)sets all game data.
// If passed NULL, allocate new game data.
struct game_t* createNewGame(struct game_t* game);
void destroyGame(struct game_t* game, bool freeMem);
void resetGame(struct game_t* game);

bool isGameComplete(struct game_t* game);
bool isInPlayingState(tap_state game);

// Adds datapoint to section data if level has incremented.
void pushCurrentState(struct game_t* game);
void addDataPointToSection(struct game_t* game, struct section_t* section, int currentLevel, int currentTime);

// Returns section data for a single section.
struct section_t* getSection(struct game_t* game, int sectionIndex);

void printGameState(struct game_t* game);

#endif /* GAME_H */
