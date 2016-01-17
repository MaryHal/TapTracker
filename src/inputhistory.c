#include "inputhistory.h"
#include "joystick.h"
#include "buttonquads.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

struct input_history_t* createInputHistory(struct input_history_t* history)
{
    if (history == NULL)
    {
        history = malloc(sizeof(struct input_history_t));
    }

    resetInputHistory(history);

    return history;
}

void destroyInputHistory(struct input_history_t* history, bool freeMe)
{
    if (freeMe)
        free(history);
}

void resetInputHistory(struct input_history_t* history)
{
    /* memset(history, 0, sizeof(struct input_history_t)); */
    history->start = 0;
    history->end = 0;

    /* pushHistoryElement(history, -1); */
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
