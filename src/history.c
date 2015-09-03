#include "history.h"
#include "joystick.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

struct history_t* createHistory(struct history_t* history)
{
    if (history == NULL)
    {
        history = (struct history_t*) malloc(sizeof(struct history_t));
    }

    resetHistory(history);

    return history;
}

void destoryHistory(struct history_t* history, bool freeMe)
{
    if (freeMe)
        free(history);
}

void resetHistory(struct history_t* history)
{
    memset(history, 0, sizeof(struct history_t));

    pushHistoryElement(history, -1);
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
    // Key Press
    if (buttonChange(joystick, BUTTON_D) && getButtonState(joystick, BUTTON_D) == GLFW_PRESS)
    {
        history->lastD = pushKey(history, 'D');
    }
    if (buttonChange(joystick, BUTTON_A) && getButtonState(joystick, BUTTON_A) == GLFW_PRESS)
    {
        history->lastA = pushKey(history, 'A');
    }
    if (buttonChange(joystick, BUTTON_B) && getButtonState(joystick, BUTTON_B) == GLFW_PRESS)
    {
        history->lastB = pushKey(history, 'B');
    }
    if (buttonChange(joystick, BUTTON_C) && getButtonState(joystick, BUTTON_C) == GLFW_PRESS)
    {
        history->lastC = pushKey(history, 'C');
    }
    if (axisChange(joystick, AXIS_HORI) && getAxisState(joystick, AXIS_HORI) == -1)
    {
        history->lastLeft = pushKey(history, 0x21d0);
    }
    if (axisChange(joystick, AXIS_HORI) && getAxisState(joystick, AXIS_HORI) == 1)
    {
        history->lastRight = pushKey(history, 0x21d2);
    }
    if (axisChange(joystick, AXIS_VERT) && getAxisState(joystick, AXIS_VERT) == -1)
    {
        history->lastUp = pushKey(history, 0x21d1);
    }
    if (axisChange(joystick, AXIS_VERT) && getAxisState(joystick, AXIS_VERT) == 1)
    {
        history->lastDown = pushKey(history, 0x21d3);
    }

    // Key Release
    if (buttonChange(joystick, BUTTON_D) && getButtonState(joystick, BUTTON_D) == GLFW_RELEASE)
    {
        if (history->lastD)
            history->lastD->held = false;
    }
    if (buttonChange(joystick, BUTTON_A) && getButtonState(joystick, BUTTON_A) == GLFW_RELEASE)
    {
        if (history->lastA)
            history->lastA->held = false;
    }
    if (buttonChange(joystick, BUTTON_B) && getButtonState(joystick, BUTTON_B) == GLFW_RELEASE)
    {
        if (history->lastB)
            history->lastB->held = false;
    }
    if (buttonChange(joystick, BUTTON_C) && getButtonState(joystick, BUTTON_C) == GLFW_RELEASE)
    {
        if (history->lastC)
            history->lastC->held = false;
    }
    if (axisChange(joystick, AXIS_HORI) && getAxisState(joystick, AXIS_HORI) != -1)
    {
        if (history->lastLeft)
            history->lastLeft->held = false;
    }
    if (axisChange(joystick, AXIS_HORI) && getAxisState(joystick, AXIS_HORI) != 1)
    {
        if (history->lastRight)
            history->lastRight->held = false;
    }
    if (axisChange(joystick, AXIS_VERT) && getAxisState(joystick, AXIS_VERT) != -1)
    {
        if (history->lastUp)
            history->lastUp->held = false;
    }
    if (axisChange(joystick, AXIS_VERT) && getAxisState(joystick, AXIS_VERT) != 1)
    {
        if (history->lastDown)
            history->lastDown->held = false;
    }
}
