#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>
#include <stdint.h>

#define JOYSTICK_AXIS_TOLERANCE 0.5f

enum
{
    BUTTON_A = 0,
    BUTTON_B,
    BUTTON_C,
    BUTTON_COUNT
};

enum
{
    AXIS_HORI = 0,
    AXIS_VERT,
    AXIS_COUNT
};

struct joystick_mapping_t
{
        int buttons[BUTTON_COUNT];
        int axisHori;
        int axisVert;
};

enum AXIS_DIRECTION
{
    AXIS_NEGATIVE = -1,
    AXIS_NEUTRAL  =  0,
    AXIS_POSITIVE =  1
};

struct joystick_t
{
        struct joystick_mapping_t jmap;
        int id;
        int buttonCount;
        int axisCount;

        uint8_t buttons[16], prevButtons[16];
        int8_t axis[8], prevAxis[8];
};

void joystick_init(struct joystick_t* js, int joystickNum, struct joystick_mapping_t jmap);
void joystick_terminate(struct joystick_t* js);

struct joystick_t* joystick_create(int joystickNum, struct joystick_mapping_t jmap);
void joystick_destroy(struct joystick_t* js);

struct joystick_t* createJoystick(struct joystick_t* joystick, int joystickNum,
                                  struct joystick_mapping_t jmap);
void destroyJoystick(struct joystick_t* joystick, bool freeMe);

void updateButtons(struct joystick_t* joystick);

uint8_t getButtonState(struct joystick_t* joystick, int buttonID);
uint8_t getPrevButtonState(struct joystick_t* joystick, int buttonID);
int8_t getAxisState(struct joystick_t* joystick, int axisID);
int8_t getPrevAxisState(struct joystick_t* joystick, int axisID);

bool buttonChange(struct joystick_t* joystick, int buttonID);
bool axisChange(struct joystick_t* joystick, int axisID);

// state should be GLFW_PRESS or GLFW_RELEASE
bool buttonChangedFromState(struct joystick_t* joystick, int buttonID, int state);
bool buttonChangedToState(struct joystick_t* joystick, int buttonID, int state);
bool axisChangedFromState(struct joystick_t* joystick, int axisID, int state);
bool axisChangedToState(struct joystick_t* joystick, int axisID, int state);

#endif /* JOYSTICK_H */
