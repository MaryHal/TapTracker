#include "joystick.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include <GLFW/glfw3.h>

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

    int buttonCount = 0;
    const unsigned char* buttons = glfwGetJoystickButtons(joystick->id, &buttonCount);
    for (int i = 0; i < buttonCount; i++)
    {
        joystick->buttons[i] = buttons[i];
    }

    int axisCount = 0;
    const float* axes = glfwGetJoystickAxes(joystick->id, &axisCount);
    for (int i = 0; i < axisCount; i++)
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
    return joystick->buttons[buttonID];
}

char getAxisState(struct joystick_t* joystick, int axisID)
{
    return joystick->axis[axisID];
}

bool buttonChange(struct joystick_t* joystick, int buttonID)
{
    return joystick->buttons[buttonID] != joystick->prevButtons[buttonID];
}

bool axisChange(struct joystick_t* joystick, int axisID)
{
    return joystick->axis[axisID] != joystick->prevAxis[axisID];
}
