#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdlib.h>

#include "history.h"

#define LEVEL_MAX      999

#define SECTION_LENGTH 100
#define SECTION_MAX    100
#define SECTION_COUNT  10

#define TIMER_FPS      60.0f
#define TAP_FPS        61.6f

#define MASTER_S9_INTERNAL_GRADE 31

float frameTimeToSeconds(int frames);
int frameTime(float seconds);

typedef enum
{
    TAP_NONE         = 0,
    TAP_START        = 1,
    TAP_ACTIVE       = 2,
    TAP_LOCKING      = 3,  // Cannot be influenced anymore
    TAP_LINECLEAR    = 4,  // Tetromino is being locked to the playfield.
    TAP_ENTRY        = 5,
    TAP_GAMEOVER     = 7,  // "Game Over" is being shown on screen.
    TAP_IDLE         = 10, // No game has started, just waiting...
    TAP_FADING       = 11, // Blocks fading away when topping out (losing).
    TAP_COMPLETION   = 13, // Blocks fading when completing the game
    TAP_STARTUP      = 71
} tap_state;

typedef enum
{
    M_NEUTRAL  = 48,

    M_FAIL_1   = 17,
    M_FAIL_2   = 19,
    M_FAIL_END = 31,

    M_PASS_1   = 49,
    M_PASS_2   = 51,
    M_SUCCESS  = 127,
} tap_mroll_flags;

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
        int endTime;
        int lines[4];
};

int getSectionTime(struct section_t* section);

struct game_t
{
        struct section_t sections[SECTION_COUNT];
        unsigned int currentSection;

        // We want to detect change on each frame, so we'll keep track of how
        // things looked on the previous frame for comparison.
        tap_state state, prevState;
        int level, prevLevel;
        int time, prevTime;

        char grade;
        char gradePoints;

        char MrollFlags;
        bool inCreditRoll;
        char sectionIndex; // reported by mame

        bool masterQualified;

        struct history_t inputHistory;
};

// (Re)sets all game data.
// If passed NULL, allocate new game data.
struct game_t* createNewGame(struct game_t* game);
void destroyGame(struct game_t* game, bool freeMem);
void resetGame(struct game_t* game);

bool isGameComplete(struct game_t* game);
bool isInPlayingState(tap_state game);

void updateGameState(struct game_t* game, int* dataPtr);

// Adds datapoint to section data if level has incremented.
void pushCurrentState(struct game_t* game);
void addDataPointToSection(struct game_t* game, struct section_t* section, int currentLevel, int currentTime);

// Returns section data for a single section.
struct section_t* getSection(struct game_t* game, int sectionIndex);

bool testMasterConditions(struct game_t* game);

// Will return true if _currently_ not invalidated from getting M-rank.
// Deprecated since we pull the flag from MAME.
bool calculateMasterConditions_(struct game_t* game);

void printGameState(struct game_t* game);

#endif /* GAME_H */
