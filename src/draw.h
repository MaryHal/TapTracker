#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>

struct game_t;
struct font_t;
struct input_history_t;
struct button_spectrum_t;
struct section_table_t;
struct game_history_t;

struct draw_data_t
{
        struct game_t* game;
        struct font_t* font;

        struct input_history_t* history;
        struct button_spectrum_t* bspec;

        struct section_table_t* table;
        struct game_history_t* gh;

        float scale;
};


typedef bool (*draw_function_p)(struct draw_data_t* data, float width, float height);

// Returns a function pointer to a draw function given a string identifier.
draw_function_p stringToDrawFunc(const char* functionIdentifier);

bool prepareDrawData(struct draw_data_t* data);

bool drawSectionGraph(struct draw_data_t* data, float width, float height);
bool drawInputHistory(struct draw_data_t* data, float width, float height);

bool drawLineCount(struct draw_data_t* data, float width, float height);
bool drawCurrentState(struct draw_data_t* data, float width, float height);

bool drawSectionTable(struct draw_data_t* data, float width, float height);
bool drawSectionTableOverall(struct draw_data_t* data, float width, float height);

bool drawGameHistory(struct draw_data_t* data, float width, float height);

bool drawMRollPassFail(struct draw_data_t* data, float width, float height);

#endif /* DRAW_H */
