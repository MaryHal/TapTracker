#ifndef DRAW_H
#define DRAW_H

struct game_t;
struct font_t;

void setupOpenGL(const unsigned int width, const unsigned int height);
void drawSectionGraph(struct game_t* game, struct font_t* font,
                      float x, float y,
                      float graphWidth, float graphHeight,
                      float margin);

#endif /* DRAW_H */
