#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>
#include <stdbool.h>

#include "joystick.h"

#define MAX_STRING_LENGTH 18
#define HISTORY_LENGTH    8

struct button_t
{
        int key;
        bool held;
};

struct element_t
{
        struct button_t spectrum[MAX_STRING_LENGTH];
        size_t size;
        int level;
};

struct history_t
{
        struct element_t data[HISTORY_LENGTH];
        int start;
        int end;

        struct button_t* heldButtons[BUTTON_COUNT];

        struct button_t* lastLeft;
        struct button_t* lastUp;
        struct button_t* lastRight;
        struct button_t* lastDown;
};

struct history_t* createHistory(struct history_t* history);
void destroyHistory(struct history_t* history, bool freeMe);
void resetHistory(struct history_t* history);

void pushHistoryElement(struct history_t* history, int level);
struct button_t* pushKey(struct history_t* history, int key);

void popHistoryElement(struct history_t* history);

struct joystick_t;
void pushCharFromJoystick(struct history_t* history, struct joystick_t* joystick);

#endif /* HISTORY_H */
