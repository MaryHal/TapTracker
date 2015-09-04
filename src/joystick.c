#include "joystick.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <math.h>

#include <GLFW/glfw3.h>

char JoystickButtonLabels[BUTTON_COUNT] =
{
    'D', 'A', 'B', 'C'
};

struct joystick_t* createJoystick(struct joystick_t* joystick, int joystickNum)
{
    if (joystick == NULL)
    {
        joystick = (struct joystick_t*) malloc(sizeof(struct joystick_t));
    }

    joystick->id = joystickNum;

    if (!glfwJoystickPresent(joystick->id))
    {
        perror("Requested joystick ID not found.");
    }

    return joystick;
}

void destroyJoystick(struct joystick_t* joystick, bool freeMe)
{
    if (freeMe)
        free(joystick);
}

void updateButtons(struct joystick_t* joystick)
{
    memcpy(joystick->prevButtons, joystick->buttons, 16);
    memcpy(joystick->prevAxis, joystick->axis, 8);

    const unsigned char* buttons = glfwGetJoystickButtons(joystick->id, &joystick->buttonCount);
    for (int i = 0; i < joystick->buttonCount; i++)
    {
        joystick->buttons[i] = buttons[i];
    }

    const float* axes = glfwGetJoystickAxes(joystick->id, &joystick->axisCount);
    for (int i = 0; i < joystick->axisCount; i++)
    {
        if (axes[i] < -0.5f)
            joystick->axis[i] = -1;
        else if (axes[i] > 0.5f)
            joystick->axis[i] = 1;
        else
            joystick->axis[i] = 0;
    }
}

unsigned char getButtonState(struct joystick_t* joystick, int buttonID)
{
    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->buttons[buttonID];
}

unsigned char getPrevButtonState(struct joystick_t* joystick, int buttonID)
{
    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->prevButtons[buttonID];
}

char getAxisState(struct joystick_t* joystick, int axisID)
{
    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->axis[axisID];
}

char getPrevAxisState(struct joystick_t* joystick, int axisID)
{
    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->prevAxis[axisID];
}

bool buttonChange(struct joystick_t* joystick, int buttonID)
{
    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->buttons[buttonID] != joystick->prevButtons[buttonID];
}

bool axisChange(struct joystick_t* joystick, int axisID)
{
    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->axis[axisID] != joystick->prevAxis[axisID];
}

bool buttonChangedFromState(struct joystick_t* joystick, int buttonID, int state)
{
    return buttonChange(joystick, buttonID) && getPrevButtonState(joystick, buttonID) == state;
}

bool buttonChangedToState(struct joystick_t* joystick, int buttonID, int state)
{
    return buttonChange(joystick, buttonID) && getButtonState(joystick, buttonID) == state;
}

bool axisChangedFromState(struct joystick_t* joystick, int axisID, int state)
{
    return axisChange(joystick, axisID) && getPrevAxisState(joystick, axisID) == state;
}

bool axisChangedToState(struct joystick_t* joystick, int axisID, int state)
{
    return axisChange(joystick, axisID) && getAxisState(joystick, axisID) == state;
}
