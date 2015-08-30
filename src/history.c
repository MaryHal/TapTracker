#include "history.h"
#include "joystick.h"

#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

struct history_t* createHistory(struct history_t* history)
{
    if (history == NULL)
    {
        history = (struct history_t*) malloc(sizeof(struct history_t));
    }

    memset(history, 0, sizeof(struct history_t));

    return history;
}

void destoryHistory(struct history_t* history, bool freeMe)
{
    if (freeMe)
        free(history);
}

void pushHistoryElement(struct history_t* history, int level)
{
    if (history->end + 1 == history->start)
    {
        popHistoryElement(history);
    }

    history->data[(history->end + 1) % HISTORY_LENGTH].level = level;
    history->end++;
}

void pushChar(struct history_t* history, char c)
{
    struct element_t* historyElement = &history->data[history->end % HISTORY_LENGTH];

    // Append char to history element string
    if (historyElement->size < MAX_STRING_LENGTH - 2)
    {
        historyElement->spectrum[historyElement->size++] = c;
        historyElement->spectrum[historyElement->size] = '\0';
    }
}

void popHistoryElement(struct history_t* history)
{
    history->start++;
}

void pushCharFromJoystick(struct history_t* history, struct joystick_t* joystick)
{
    if (buttonChange(joystick, BUTTON_D) && getButtonState(joystick, BUTTON_D) == GLFW_PRESS)
    {
        pushChar(history, 'D');
    }
    if (buttonChange(joystick, BUTTON_A) && getButtonState(joystick, BUTTON_A) == GLFW_PRESS)
    {
        pushChar(history, 'A');
    }
    if (buttonChange(joystick, BUTTON_B) && getButtonState(joystick, BUTTON_B) == GLFW_PRESS)
    {
        pushChar(history, 'B');
    }
    if (buttonChange(joystick, BUTTON_C) && getButtonState(joystick, BUTTON_C) == GLFW_PRESS)
    {
        pushChar(history, 'C');
    }
}
