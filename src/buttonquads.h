#ifndef BUTTONQUADS_H
#define BUTTONQUADS_H

#include <stdbool.h>
#include <stdint.h>

#include "joystick.h"

#define HELD_BUTTON_OFFSET 8

enum
{
    BUTTON_QUAD_BLANK = 0,
    BUTTON_QUAD_UP = 1,
    BUTTON_QUAD_DOWN,
    BUTTON_QUAD_LEFT,
    BUTTON_QUAD_RIGHT,
    BUTTON_QUAD_A,
    BUTTON_QUAD_B,
    BUTTON_QUAD_C,
    BUTTON_QUAD_D,
    BUTTON_QUAD_UP_HELD,
    BUTTON_QUAD_DOWN_HELD,
    BUTTON_QUAD_LEFT_HELD,
    BUTTON_QUAD_RIGHT_HELD,
    BUTTON_QUAD_A_HELD,
    BUTTON_QUAD_B_HELD,
    BUTTON_QUAD_C_HELD,
    BUTTON_QUAD_D_HELD,
    BUTTON_QUAD_COUNT
};

struct button_spectrum_quad_t
{
        float texCoords[8];
};

struct button_spectrum_t
{
        unsigned int textureID;
        struct button_spectrum_quad_t quads[BUTTON_QUAD_COUNT];
};

void button_spectrum_init(struct button_spectrum_t* bspec, const uint8_t* data, size_t size);
void button_spectrum_terminate(struct button_spectrum_t* bspec);

struct button_spectrum_t* button_spectrum_create(const uint8_t* data, size_t size);
void button_spectrum_destroy(struct button_spectrum_t* bspec);

uint8_t joystickButtonToQuadIndex(struct joystick_mapping_t jmap, uint8_t button);

#endif /* BUTTONQUADS_H */
