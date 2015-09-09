#ifndef DRAW_H
#define DRAW_H

struct game_t;
struct font_t;
struct history_t;

void setupOpenGL(const unsigned int width, const unsigned int height);

void drawSectionGraph(struct game_t* game, struct font_t* font,
                      float width, float height);

void drawHistory(struct game_t* game, struct font_t* font,
                 float width, float height);

void drawSectionLineCount(struct game_t* game, struct font_t* font,
                          float width, float height);

#endif /* DRAW_H */
