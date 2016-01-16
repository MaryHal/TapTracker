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
    for (int buttonID = 0; buttonID < BUTTON_COUNT; buttonID++)
    {
        if (buttonChange(joystick, buttonID))
        {
            if (getButtonState(joystick, buttonID) == GLFW_PRESS)
            {
                history->heldButtons[buttonID] = pushKey(history, joystickButtonToSheetIndex(joystick->jmap, buttonID));
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
        history->lastLeft = pushKey(history, BUTTON_INDEX_LEFT);
    }
    if (axisChangedToState(joystick, joystick->jmap.axisHori, AXIS_POSITIVE))
    {
        history->lastRight = pushKey(history, BUTTON_INDEX_RIGHT);
    }
    if (axisChangedToState(joystick, joystick->jmap.axisVert, AXIS_NEGATIVE))
    {
        history->lastUp = pushKey(history, BUTTON_INDEX_UP);
    }
    if (axisChangedToState(joystick, joystick->jmap.axisVert, AXIS_POSITIVE))
    {
        history->lastDown = pushKey(history, BUTTON_INDEX_DOWN);
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

uint8_t joystickButtonToSheetIndex(struct joystick_mapping_t jmap, uint8_t button)
{
    if (button == jmap.buttonD)
        return BUTTON_INDEX_D;
    if (button == jmap.buttonA)
        return BUTTON_INDEX_A;
    if (button == jmap.buttonB)
        return BUTTON_INDEX_B;
    if (button == jmap.buttonC)
        return BUTTON_INDEX_C;

    return BUTTON_INDEX_BLANK;
}
