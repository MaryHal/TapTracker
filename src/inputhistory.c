#include "inputhistory.h"
#include "joystick.h"
#include "buttonquads.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <GLFW/glfw3.h>

static struct button_t EMPTY_BUTTON_T = { .jkey = 0, .held = false };

void input_history_init(struct input_history_t* ihist)
{
    resetInputHistory(ihist);
}

void input_history_terminate(struct input_history_t* ihist)
{
    (void) ihist;
}

struct input_history_t* input_history_create()
{
    struct input_history_t* ihist = malloc(sizeof(struct input_history_t));
    if (ihist)
    {
        input_history_init(ihist);
    }

    return ihist;
}

void input_history_destroy(struct input_history_t* ihist)
{
    assert(ihist != NULL);

    input_history_terminate(ihist);
    free(ihist);
}

void resetInputHistory(struct input_history_t* history)
{
    /* memset(history, 0, sizeof(struct input_history_t)); */
    history->start = 0;
    history->end = 0;

    history->heldButtons[0] = &EMPTY_BUTTON_T;
    history->heldButtons[1] = &EMPTY_BUTTON_T;
    history->heldButtons[2] = &EMPTY_BUTTON_T;

    history->lastLeft  = &EMPTY_BUTTON_T;
    history->lastUp    = &EMPTY_BUTTON_T;
    history->lastRight = &EMPTY_BUTTON_T;
    history->lastDown  = &EMPTY_BUTTON_T;
}

void pushInputHistoryElement(struct input_history_t* history, int level)
{
    if (history->end - history->start == INPUT_HISTORY_LENGTH)
    {
        popInputHistoryElement(history);
    }

    const size_t elementIndex = (history->end) % INPUT_HISTORY_LENGTH;

    history->data[elementIndex].level = level;
    history->data[elementIndex].size = 0;
    history->end++;
}

struct button_t* pushKey(struct input_history_t* history, int key)
{
    // Don't push if we're empty!
    if (history->start == history->end)
    {
        return NULL;
    }

    struct element_t* inputhistoryElement = &history->data[(history->end - 1) % INPUT_HISTORY_LENGTH];

    // Append button to input history element string
    if (inputhistoryElement->size < MAX_INPUT_STRING_LENGTH - 2)
    {
        inputhistoryElement->spectrum[inputhistoryElement->size] = (struct button_t){ key, true };
        struct button_t* ret = &inputhistoryElement->spectrum[inputhistoryElement->size];
        inputhistoryElement->size++;

        return ret;
    }

    return NULL;
}

void popInputHistoryElement(struct input_history_t* history)
{
    history->start++;
}

struct element_t* getInputHistoryElement(struct input_history_t* inputhistory, int index)
{
    assert(index >= inputhistory->start);
    assert(index <  inputhistory->end);
    return &inputhistory->data[index % INPUT_HISTORY_LENGTH];
}

void pushInputFromJoystick(struct input_history_t* history, struct joystick_t* joystick)
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        int buttonID = joystick->jmap.buttons[i];
        if (buttonChange(joystick, buttonID))
        {
            if (getButtonState(joystick, buttonID) == GLFW_PRESS)
            {
                history->heldButtons[buttonID] = pushKey(history, joystickButtonToQuadIndex(joystick->jmap, buttonID));
            }
            else if (getButtonState(joystick, buttonID) == GLFW_RELEASE)
            {
                if (history->heldButtons[buttonID])
                    history->heldButtons[buttonID]->held = false;
            }
        }
    }

    if (axisChangedToState(joystick, joystick->jmap.axisHori, AXIS_NEGATIVE))
    {
        history->lastLeft = pushKey(history, BUTTON_QUAD_LEFT);
    }
    if (axisChangedToState(joystick, joystick->jmap.axisHori, AXIS_POSITIVE))
    {
        history->lastRight = pushKey(history, BUTTON_QUAD_RIGHT);
    }
    if (axisChangedToState(joystick, joystick->jmap.axisVert, AXIS_NEGATIVE))
    {
        history->lastUp = pushKey(history, BUTTON_QUAD_UP);
    }
    if (axisChangedToState(joystick, joystick->jmap.axisVert, AXIS_POSITIVE))
    {
        history->lastDown = pushKey(history, BUTTON_QUAD_DOWN);
    }
    if (axisChangedFromState(joystick, joystick->jmap.axisHori, AXIS_NEGATIVE))
    {
        if (history->lastLeft)
            history->lastLeft->held = false;
    }
    if (axisChangedFromState(joystick, joystick->jmap.axisHori, AXIS_POSITIVE))
    {
        if (history->lastRight)
            history->lastRight->held = false;
    }
    if (axisChangedFromState(joystick, joystick->jmap.axisVert, AXIS_NEGATIVE))
    {
        if (history->lastUp)
            history->lastUp->held = false;
    }
    if (axisChangedFromState(joystick, joystick->jmap.axisVert, AXIS_POSITIVE))
    {
        if (history->lastDown)
            history->lastDown->held = false;
    }
}
