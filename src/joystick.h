#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>

// TODO: Button profiles. I hard-coded my joystick info for now.

enum MyButtons
{
    BUTTON_D = 0,
    BUTTON_A = 1,
    BUTTON_B = 2,
    BUTTON_C = 3
};

enum MyAxis
{
    AXIS_HORI = 6,
    AXIS_VERT = 7
};

struct joystick_t
{
        int id;
        unsigned char buttons[16], prevButtons[16];
        float axis[8], prevAxis[8];
};

struct joystick_t* createJoystick(struct joystick_t* joystick, int joystickNum);
void destroyJoystick(struct joystick_t* joystick, bool freeMe);

void updateButtons(struct joystick_t* joystick);

unsigned char getButtonState(struct joystick_t* joystick, int buttonID);
float getAxisState(struct joystick_t* joystick, int axisID);

bool buttonChange(struct joystick_t* joystick, int buttonID);
bool axisChange(struct joystick_t* joystick, int axisID);

#endif /* JOYSTICK_H */
