#ifndef DRAW_H
#define DRAW_H

struct game_t;
struct font_t;
struct history_t;

void setupOpenGL(const unsigned int width, const unsigned int height);
void drawSectionGraph(struct game_t* game, struct font_t* font,
                      float scale,
                      float x, float y,
                      float graphWidth, float graphHeight,
                      float margin);

void drawHistory(struct history_t* history, struct font_t* font,
                 float x, float y,
                 float margin);

void drawSectionLineCount(struct game_t* game, struct font_t* font,
                          float x, float y,
                          float margin);

#endif /* DRAW_H */
