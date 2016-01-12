#ifndef DRAW_H
#define DRAW_H

struct game_t;
struct font_t;
struct history_t;
struct button_spectrum_t;

struct draw_data_t
{
        struct game_t* game;
        struct font_t* font;
        struct history_t* history;
        struct button_spectrum_t* bspec;

        float scale;
};

void setupOpenGL(const unsigned int width, const unsigned int height);

void drawSectionGraph(struct draw_data_t* data, float width, float height);

void drawInputHistory(struct draw_data_t* data, float width, float height);

void drawSectionTable(struct draw_data_t* data, float width, float height);

#endif /* DRAW_H */
