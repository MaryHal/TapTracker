#ifndef INPUTHISTORY_H
#define INPUTHISTORY_H

#include <stddef.h>
#include <stdbool.h>

#include "joystick.h"

#define MAX_INPUT_STRING_LENGTH 14
#define INPUT_HISTORY_LENGTH    10

struct button_t
{
        uint8_t jkey;
        bool held;
};

struct element_t
{
        struct button_t spectrum[MAX_INPUT_STRING_LENGTH];
        size_t size;
        int level;
};

struct input_history_t
{
        struct element_t data[INPUT_HISTORY_LENGTH];
        int start;
        int end;

        struct button_t* heldButtons[BUTTON_COUNT];

        struct button_t* lastLeft;
        struct button_t* lastUp;
        struct button_t* lastRight;
        struct button_t* lastDown;
};

extern struct button_t EMPTY_BUTTON_T;

void input_history_init(struct input_history_t* ihist);
void input_history_terminate(struct input_history_t* ihist);

struct input_history_t* input_history_create();
void input_history_destroy(struct input_history_t* ihist);

struct input_history_t* createInputHistory(struct input_history_t* inputhistory);
void destroyInputHistory(struct input_history_t* inputhistory, bool freeMe);
void resetInputHistory(struct input_history_t* inputhistory);

void pushInputHistoryElement(struct input_history_t* inputhistory, int level);
struct button_t* pushKey(struct input_history_t* inputhistory, int key);

void popInputHistoryElement(struct input_history_t* inputhistory);

// Given an index between start and end, return the associated data element.
struct element_t* getInputHistoryElement(struct input_history_t* inputhistory, int index);

void pushInputFromJoystick(struct input_history_t* inputhistory, struct joystick_t* joystick);

#endif /* INPUTHISTORY_H */
