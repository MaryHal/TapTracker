#ifndef INPUTHISTORY_H
#define INPUTHISTORY_H

#include <stdbool.h>

#define HELD_BUTTON_OFFSET 8

enum
{
    INPUT_BLANK = 0,
    INPUT_UP = 1,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_A,
    INPUT_B,
    INPUT_C,
    INPUT_D,
    INPUT_UP_HELD,
    INPUT_DOWN_HELD,
    INPUT_LEFT_HELD,
    INPUT_RIGHT_HELD,
    INPUT_A_HELD,
    INPUT_B_HELD,
    INPUT_C_HELD,
    INPUT_D_HELD,
    INPUT_COUNT
};

struct button_spectrum_quad_t
{
        float texCoords[8];
};

struct button_spectrum_t
{
        unsigned int textureID;
        struct button_spectrum_quad_t quads[INPUT_COUNT];
};

struct button_spectrum_t* createButtonSheet(struct button_spectrum_t* bspec);
void destroyButtonSheet(struct button_spectrum_t* bspec, bool freeMe);

#endif /* INPUTHISTORY_H */
