#ifndef DRAW_H
#define DRAW_H

struct game_t;

void setupOpenGL(const unsigned int width, const unsigned int height);
void drawSectionGraph(struct game_t* game, unsigned int x, unsigned int y,
                      unsigned int graphWidth, unsigned int graphHeight);

#endif /* DRAW_H */
