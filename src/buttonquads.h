#ifndef BUTTONQUADS_H
#define BUTTONQUADS_H

#include <stdbool.h>

#define HELD_BUTTON_OFFSET 8

enum
{
    BUTTON_INDEX_BLANK = 0,
    BUTTON_INDEX_UP = 1,
    BUTTON_INDEX_DOWN,
    BUTTON_INDEX_LEFT,
    BUTTON_INDEX_RIGHT,
    BUTTON_INDEX_A,
    BUTTON_INDEX_B,
    BUTTON_INDEX_C,
    BUTTON_INDEX_D,
    BUTTON_INDEX_UP_HELD,
    BUTTON_INDEX_DOWN_HELD,
    BUTTON_INDEX_LEFT_HELD,
    BUTTON_INDEX_RIGHT_HELD,
    BUTTON_INDEX_A_HELD,
    BUTTON_INDEX_B_HELD,
    BUTTON_INDEX_C_HELD,
    BUTTON_INDEX_D_HELD,
    BUTTON_INDEX_COUNT
};

struct button_spectrum_quad_t
{
        float texCoords[8];
};

struct button_spectrum_t
{
        unsigned int textureID;
        struct button_spectrum_quad_t quads[BUTTON_INDEX_COUNT];
};

struct button_spectrum_t* createButtonSpriteSheet(struct button_spectrum_t* bspec);
void destroyButtonSpriteSheet(struct button_spectrum_t* bspec, bool freeMe);

#endif /* BUTTONQUADS_H */
