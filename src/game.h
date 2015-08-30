#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdlib.h>

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
    LOCKED       = 4, // Tetromino is being locked to the playfield.
    ENTRY        = 5,
    GAMEOVER     = 7,
    IDLE         = 10, // No game has started, just waiting...
    FADING       = 11, // Blocks fading away when topping out
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

        tap_state state;
        tap_state prevState;

        int currentLevel;
        int currentTime;
};

float convertTime(int frames);

// (Re)sets all game data.
// If passed NULL, allocate new game data.
struct game_t* createNewGame(struct game_t* game);
void destroyGame(struct game_t* game, bool freeMem);
void resetGame(struct game_t* game);

bool isGameComplete(struct game_t* game);

void updateState(struct game_t* game, tap_state newState);
bool stateChangedTo(struct game_t* game, tap_state testState);
bool stateChangedFrom(struct game_t* game, tap_state testState);

bool isInPlayingState(struct game_t* game);

// Adds datapoint to section data if level has incremented.
void pushDataPoint(struct game_t* game, struct datapoint_t datapoint);
void pushDataPointToSection(struct section_t* section, struct datapoint_t datapoint);

// Returns section data for a single section.
struct section_t* getSection(struct game_t* game, int sectionIndex);

#endif /* GAME_H */
