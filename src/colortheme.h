#ifndef COLORTHEME_H
#define COLORTHEME_H

enum COLOR_INDEX
{
    COLOR_SINGLE = 0,
    COLOR_DOUBLE = 1,
    COLOR_TRIPLE = 2,
    COLOR_TETRIS = 3,
    COLOR_FOREGROUND = 4,
    COLOR_BACKGROUND = 5,
    NUM_COLORS = 6
};

extern float DARK_THEME[NUM_COLORS][3];
extern float LIGHT_THEME[NUM_COLORS][3];

extern float (*colortheme)[NUM_COLORS][3];

void setColorTheme(float (*theme)[NUM_COLORS][3]);
void setGLColor(unsigned int colorIndex, float alpha);
void setGLClearColor();

void hexToRGB(float* rgbOutput, unsigned int hex);

#endif /* COLORTHEME_H */
