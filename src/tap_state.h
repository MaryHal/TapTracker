#ifndef TAP_STATE_H
#define TAP_STATE_H

#include <stdint.h>

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
};

#endif /* TAP_STATE_H */
