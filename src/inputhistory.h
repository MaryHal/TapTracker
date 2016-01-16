#ifndef INPUTHISTORY_H
#define INPUTHISTORY_H

#include <stddef.h>
#include <stdbool.h>

#include "joystick.h"

#define MAX_INPUT_STRING_LENGTH 10
#define INPUT_HISTORY_LENGTH    10

struct button_t
{
        int8_t jkey;
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

struct input_history_t* createInputHistory(struct input_history_t* inputhistory);
void destroyInputHistory(struct input_history_t* inputhistory, bool freeMe);
void resetInputHistory(struct input_history_t* inputhistory);

void pushInputHistoryElement(struct input_history_t* inputhistory, int level);
struct button_t* pushKey(struct input_history_t* inputhistory, int key);

void popInputHistoryElement(struct input_history_t* inputhistory);

void pushInputFromJoystick(struct input_history_t* inputhistory, struct joystick_t* joystick);

uint8_t joystickButtonToSheetIndex(struct joystick_mapping_t jmap, uint8_t button);

#endif /* INPUTHISTORY_H */
