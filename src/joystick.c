#include "joystick.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <zf_log.h>

#include <math.h>

#include <GLFW/glfw3.h>

void joystick_init(struct joystick_t* js, int joystickNum, struct joystick_mapping_t jmap)
{
    if (!glfwJoystickPresent(joystickNum))
    {
        ZF_LOGE("Requested joystick ID not found.");
        return;
    }

    js->jmap = jmap;
    js->id = joystickNum;
    js->buttonCount = 0;
    js->axisCount = 0;
}

void joystick_terminate(struct joystick_t* js)
{
    (void) js;
}

struct joystick_t* joystick_create(int joystickNum, struct joystick_mapping_t jmap)
{
    struct joystick_t* js = malloc(sizeof(struct joystick_t));
    if (js)
    {
        joystick_init(js, joystickNum, jmap);
    }

    return js;
}

void joystick_destroy(struct joystick_t* js)
{
    assert(js != NULL);

    joystick_terminate(js);
    free(js);
}

void updateButtons(struct joystick_t* joystick)
{
    assert(joystick != NULL);

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
        if (axes[i] < -JOYSTICK_AXIS_TOLERANCE)
            joystick->axis[i] = -1;
        else if (axes[i] > JOYSTICK_AXIS_TOLERANCE)
            joystick->axis[i] = 1;
        else
            joystick->axis[i] = 0;
    }
}

uint8_t getButtonState(struct joystick_t* joystick, int buttonID)
{
    assert(joystick != NULL);

    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->buttons[buttonID];
}

uint8_t getPrevButtonState(struct joystick_t* joystick, int buttonID)
{
    assert(joystick != NULL);

    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->prevButtons[buttonID];
}

int8_t getAxisState(struct joystick_t* joystick, int axisID)
{
    assert(joystick != NULL);

    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->axis[axisID];
}

int8_t getPrevAxisState(struct joystick_t* joystick, int axisID)
{
    assert(joystick != NULL);

    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->prevAxis[axisID];
}

bool buttonChange(struct joystick_t* joystick, int buttonID)
{
    assert(joystick != NULL);

    assert(buttonID >= 0 && buttonID < joystick->buttonCount);
    return joystick->buttons[buttonID] != joystick->prevButtons[buttonID];
}

bool axisChange(struct joystick_t* joystick, int axisID)
{
    assert(joystick != NULL);

    assert(axisID >= 0 && axisID < joystick->axisCount);
    return joystick->axis[axisID] != joystick->prevAxis[axisID];
}

bool buttonChangedFromState(struct joystick_t* joystick, int buttonID, int state)
{
    assert(joystick != NULL);

    return buttonChange(joystick, buttonID) && getPrevButtonState(joystick, buttonID) == state;
}

bool buttonChangedToState(struct joystick_t* joystick, int buttonID, int state)
{
    assert(joystick != NULL);

    return buttonChange(joystick, buttonID) && getButtonState(joystick, buttonID) == state;
}

bool axisChangedFromState(struct joystick_t* joystick, int axisID, int state)
{
    assert(joystick != NULL);

    return axisChange(joystick, axisID) && getPrevAxisState(joystick, axisID) == state;
}

bool axisChangedToState(struct joystick_t* joystick, int axisID, int state)
{
    assert(joystick != NULL);

    return axisChange(joystick, axisID) && getAxisState(joystick, axisID) == state;
}
