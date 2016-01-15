#ifndef TAP_STATE_H
#define TAP_STATE_H

#include <stdint.h>

enum tap_internal_state
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
};

enum tap_mroll_flags
{
    M_FAIL_1   = 17,
    M_FAIL_2   = 19,
    M_FAIL_END = 31,

    M_NEUTRAL  = 48,
    M_PASS_1   = 49,
    M_PASS_2   = 51,
    M_SUCCESS  = 127,
};

enum tap_game_mode
{
    TAP_MODE_NULL           = 0,
    TAP_MODE_NORMAL         = 1,
    TAP_MODE_MASTER         = 2,
    TAP_MODE_DOUBLES        = 4,
    TAP_MODE_NORMAL_VERSUS  = 9,
    TAP_MODE_MASTER_VERSUS  = 10,
    TAP_MODE_MASTER_CREDITS = 18,
    TAP_MODE_TGMPLUS_VERSUS = 136,
    TAP_MODE_TGMPLUS        = 128,
    TAP_MODE_MASTER_ITEM    = 514,
    TAP_MODE_TGMPLUS_ITEM   = 640,
    TAP_MODE_DEATH          = 4096,
    TAP_MODE_DEATH_VERSUS   = 4104
};

struct tap_state
{
        int16_t state;
        int16_t grade;
        int16_t gradePoints;

        int16_t level;
        int16_t timer;

        int16_t tetromino;
        int16_t xcoord;
        int16_t ycoord;
        int16_t rotation;
        int16_t mrollFlags;
        int16_t inCreditRoll;

        int16_t gameMode;
};

#endif /* TAP_STATE_H */
