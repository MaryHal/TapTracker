#include "joystick.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <math.h>

#include <GLFW/glfw3.h>

struct joystick_t* createJoystick(struct joystick_t* joystick, int joystickNum,
                                  struct joystick_mapping_t jmap)
{
    if (!glfwJoystickPresent(joystickNum))
    {
        perror("Requested joystick ID not found");
        return NULL;
    }

    if (joystick == NULL)
    {
        joystick = malloc(sizeof(struct joystick_t));
    }

    joystick->jmap = jmap;
    joystick->id = joystickNum;
    joystick->buttonCount = 0;
    joystick->axisCount = 0;

    return joystick;
}

void destroyJoystick(struct joystick_t* joystick, bool freeMe)
{
    if (freeMe)
        free(joystick);
}

void updateButtons(struct joystick_t* joystick)
{
    if (!joystick)
        return;

    memcpy(joystick->prevButtons, joystick->buttons, 16);
    memcpy(joystick->prevAxis, joystick->axis, 8);

    const uint8_t* buttons = glfwGetJoystickButtons(joystick->id, &joystick->buttonCount);
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

uint8_t getButtonState(struct joystick_t* joystick, int buttonID)
{
    if (!joystick)
        return 0;

    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->buttons[buttonID];
}

uint8_t getPrevButtonState(struct joystick_t* joystick, int buttonID)
{
    if (!joystick)
        return 0;

    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->prevButtons[buttonID];
}

int8_t getAxisState(struct joystick_t* joystick, int axisID)
{
    if (!joystick)
        return 0;

    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->axis[axisID];
}

int8_t getPrevAxisState(struct joystick_t* joystick, int axisID)
{
    if (!joystick)
        return 0;

    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->prevAxis[axisID];
}

bool buttonChange(struct joystick_t* joystick, int buttonID)
{
    if (!joystick)
        return 0;

    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->buttons[buttonID] != joystick->prevButtons[buttonID];
}

bool axisChange(struct joystick_t* joystick, int axisID)
{
    if (!joystick)
        return 0;

    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->axis[axisID] != joystick->prevAxis[axisID];
}

bool buttonChangedFromState(struct joystick_t* joystick, int buttonID, int state)
{
    if (!joystick)
        return 0;

    return buttonChange(joystick, buttonID) && getPrevButtonState(joystick, buttonID) == state;
}

bool buttonChangedToState(struct joystick_t* joystick, int buttonID, int state)
{
    if (!joystick)
        return 0;

    return buttonChange(joystick, buttonID) && getButtonState(joystick, buttonID) == state;
}

bool axisChangedFromState(struct joystick_t* joystick, int axisID, int state)
{
    if (!joystick)
        return 0;

    return axisChange(joystick, axisID) && getPrevAxisState(joystick, axisID) == state;
}

bool axisChangedToState(struct joystick_t* joystick, int axisID, int state)
{
    if (!joystick)
        return 0;

    return axisChange(joystick, axisID) && getAxisState(joystick, axisID) == state;
}
