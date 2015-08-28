#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdlib.h>

#define SECTION_MAX 100
#define SECTION_COUNT 10

typedef enum
{
    NONE    = 0,
    /* PRE_ENTRY = 1, */
    ACTIVE  = 2,
    LOCKING = 3, // Cannot be influenced anymore
    LOCKED  = 4, // Tetromino is being locked to the playfield.
    ENTRY   = 5,
    IDLE    = 10,
    STARTUP = 71
} tap_state;

bool isInPlayingState(tap_state state);
float convertTime(int frames);

struct datapoint_t
{
        int level;
        int time;
};

struct section_t
{
        struct datapoint_t data[SECTION_MAX];
        size_t size;

        int lines[4];
};

struct game_t
{
        struct section_t sections[SECTION_COUNT];
};

// (Re)sets all game data.
// If passed NULL, allocate new game data.
struct game_t* createNewGame(struct game_t* game);

// Adds datapoint to section data if level has incremented.
void pushDataPoint(struct game_t* game, struct datapoint_t datapoint);
void pushDataPointToSection(struct section_t* section, struct datapoint_t datapoint);

// Returns section data for a single section.
struct section_t* getSection(struct game_t* game, int sectionIndex);

#endif /* GAME_H */
