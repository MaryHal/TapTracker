#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>

// TODO: Button profiles. I hard-coded my joystick info for now.

enum MyButtons
{
    BUTTON_D = 0,
    BUTTON_A = 1,
    BUTTON_B = 2,
    BUTTON_C = 3,
    BUTTON_COUNT = 4
};

enum MyAxis
{
    AXIS_HORI = 6,
    AXIS_VERT = 7
};

enum AXIS_DIRECTION
{
    AXIS_NEGATIVE = -1,
    AXIS_NEUTRAL  =  0,
    AXIS_POSITIVE =  1
};

struct joystick_t
{
        int id;
        int buttonCount;
        int axisCount;
        unsigned char buttons[16], prevButtons[16];
        char axis[8], prevAxis[8];
};

struct joystick_t* createJoystick(struct joystick_t* joystick, int joystickNum);
void destroyJoystick(struct joystick_t* joystick, bool freeMe);

void updateButtons(struct joystick_t* joystick);

unsigned char getButtonState(struct joystick_t* joystick, int buttonID);
unsigned char getPrevButtonState(struct joystick_t* joystick, int buttonID);
char getAxisState(struct joystick_t* joystick, int axisID);
char getPrevAxisState(struct joystick_t* joystick, int axisID);

bool buttonChange(struct joystick_t* joystick, int buttonID);
bool axisChange(struct joystick_t* joystick, int axisID);

bool buttonChangedFromState(struct joystick_t* joystick, int buttonID, int state); // state should be GLFW_PRESS or GLFW_RELEASE
bool buttonChangedToState(struct joystick_t* joystick, int buttonID, int state); // state should be GLFW_PRESS or GLFW_RELEASE
bool axisChangedFromState(struct joystick_t* joystick, int axisID, int state);
bool axisChangedToState(struct joystick_t* joystick, int axisID, int state);

#endif /* JOYSTICK_H */
