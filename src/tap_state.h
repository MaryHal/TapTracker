#ifndef TAP_STATE_H
#define TAP_STATE_H

#include <stdint.h>

#define TIMER_FPS      60.0f
#define TAP_FPS        61.618f

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

#define MROLL_PASS_MASK (1 << 5)

enum tap_game_mode
{
    TAP_MODE_NULL           = 0,
    TAP_MODE_NORMAL         = 1,
    TAP_MODE_MASTER         = 2,
    TAP_MODE_DOUBLES        = 4,
    TAP_MODE_NORMAL_VERSUS  = 9,
    TAP_MODE_MASTER_VERSUS  = 10,
    TAP_MODE_MASTER_CREDITS = 18,
    TAP_MODE_NORMAL_20G     = 33,
    TAP_MODE_MASTER_20G     = 34,
    TAP_MODE_DOUBLES_20G    = 36,
    TAP_MODE_TGMPLUS        = 128,
    TAP_MODE_TGMPLUS_VERSUS = 136,
    TAP_MODE_TGMPLUS_20G    = 160,
    TAP_MODE_MASTER_ITEM    = 514,
    TAP_MODE_TGMPLUS_ITEM   = 640,
    TAP_MODE_DEATH          = 4096,
    TAP_MODE_DEATH_VERSUS   = 4104,
    TAP_MODE_DEATH_20G      = 4128
};

#define MODE_VERSUS_MASK  (1 << 3)
#define MODE_CREDITS_MASK (1 << 4)
#define MODE_20G_MASK     (1 << 5)
#define MODE_BIG_MASK     (1 << 6)
#define MODE_ITEM_MASK    (1 << 9)
#define MODE_TLS_MASK     (1 << 10)

struct tap_state
{
        int16_t state;
        int16_t grade;
        int16_t gradePoints;

        int16_t level;
        int16_t timer; // Timer value in frames. Runs at 60 fps.

        int16_t tetromino;
        int16_t xcoord;
        int16_t ycoord;
        int16_t rotation;
        int16_t mrollFlags;
        int16_t inCreditRoll;

        int16_t gameMode;
};

#endif /* TAP_STATE_H */
