#include "history.h"
#include "joystick.h"
#include "buttonquads.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

struct history_t* createHistory(struct history_t* history)
{
    if (history == NULL)
    {
        history = malloc(sizeof(struct history_t));
    }

    resetHistory(history);

    return history;
}

void destroyHistory(struct history_t* history, bool freeMe)
{
    if (freeMe)
        free(history);
}

void resetHistory(struct history_t* history)
{
    memset(history, 0, sizeof(struct history_t));

    /* pushHistoryElement(history, -1); */
}

void pushHistoryElement(struct history_t* history, int level)
{
    if (history->end - history->start == HISTORY_LENGTH)
    {
        popHistoryElement(history);
    }

    const size_t elementIndex = (history->end) % HISTORY_LENGTH;

    history->data[elementIndex].level = level;
    history->data[elementIndex].size = 0;
    history->end++;
}

struct button_t* pushKey(struct history_t* history, int key)
{
    // Don't push if we're empty!
    if (history->start == history->end)
    {
        return NULL;
    }

    struct element_t* historyElement = &history->data[(history->end - 1) % HISTORY_LENGTH];

    // Append char to history element string
    if (historyElement->size < MAX_STRING_LENGTH - 2)
    {
        historyElement->spectrum[historyElement->size] = (struct button_t){ key, true };
        struct button_t* ret = &historyElement->spectrum[historyElement->size];
        historyElement->size++;

        return ret;
    }

    return NULL;
}

void popHistoryElement(struct history_t* history)
{
    history->start++;
}

void pushCharFromJoystick(struct history_t* history, struct joystick_t* joystick)
{
    for (int buttonID = 0; buttonID < BUTTON_COUNT; buttonID++)
    {
        if (buttonChange(joystick, buttonID))
        {
            if (getButtonState(joystick, buttonID) == GLFW_PRESS)
            {
                history->heldButtons[buttonID] = pushKey(history, joystickButtonToSheetIndex(buttonID));
            }
            else if (getButtonState(joystick, buttonID) == GLFW_RELEASE)
            {
                if (history->heldButtons[buttonID])
                    history->heldButtons[buttonID]->held = false;
            }
        }
    }

    if (axisChangedToState(joystick, AXIS_HORI, AXIS_NEGATIVE))
    {
        history->lastLeft = pushKey(history, BUTTON_INDEX_LEFT);
    }
    if (axisChangedToState(joystick, AXIS_HORI, AXIS_POSITIVE))
    {
        history->lastRight = pushKey(history, BUTTON_INDEX_RIGHT);
    }
    if (axisChangedToState(joystick, AXIS_VERT, AXIS_NEGATIVE))
    {
        history->lastUp = pushKey(history, BUTTON_INDEX_UP);
    }
    if (axisChangedToState(joystick, AXIS_VERT, AXIS_POSITIVE))
    {
        history->lastDown = pushKey(history, BUTTON_INDEX_DOWN);
    }
    if (axisChangedFromState(joystick, AXIS_HORI, AXIS_NEGATIVE))
    {
        if (history->lastLeft)
            history->lastLeft->held = false;
    }
    if (axisChangedFromState(joystick, AXIS_HORI, AXIS_POSITIVE))
    {
        if (history->lastRight)
            history->lastRight->held = false;
    }
    if (axisChangedFromState(joystick, AXIS_VERT, AXIS_NEGATIVE))
    {
        if (history->lastUp)
            history->lastUp->held = false;
    }
    if (axisChangedFromState(joystick, AXIS_VERT, AXIS_POSITIVE))
    {
        if (history->lastDown)
            history->lastDown->held = false;
    }
}

unsigned int joystickButtonToSheetIndex(unsigned int button)
{
    switch (button)
    {
    case BUTTON_D:
        return BUTTON_INDEX_D;
    case BUTTON_A:
        return BUTTON_INDEX_A;
    case BUTTON_B:
        return BUTTON_INDEX_B;
    case BUTTON_C:
        return BUTTON_INDEX_C;
    default:
        return BUTTON_INDEX_BLANK;
    };
}
