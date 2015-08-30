#include "history.h"

#include <stdlib.h>
#include <string.h>

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

void pushNewElement(struct history_t* history, int level)
{
    if (history->end == history->start)
    {
        popElement(history);
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

void popElement(struct history_t* history)
{
    history->start++;
}
