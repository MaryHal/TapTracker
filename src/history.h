#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_STRING_LENGTH 16
#define HISTORY_LENGTH    16

struct element_t
{
        char spectrum[MAX_STRING_LENGTH];
        size_t size;
        int level;
};

struct history_t
{
        struct element_t data[HISTORY_LENGTH];
        size_t start;
        size_t end;
};

struct history_t* createHistory(struct history_t* history);
void destoryHistory(struct history_t* history, bool freeMe);

void pushHistoryElement(struct history_t* history, int level);
void pushChar(struct history_t* history, char c);

void popHistoryElement(struct history_t* history);

struct joystick_t;
void pushCharFromJoystick(struct history_t* history, struct joystick_t* joystick);
void printHistory(struct history_t* history);

#endif /* HISTORY_H */
