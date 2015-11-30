#ifndef COLORTHEME_H
#define COLORTHEME_H

extern float DARK_THEME[6][3];
extern float LIGHT_THEME[6][3];

extern float (*colortheme)[6][3];

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

void setColorTheme(float (*theme)[6][3]);
void setGLColor(unsigned int colorIndex, float alpha);
void setGLClearColor();

void hexToRGB(float* rgbOutput, unsigned int hex);

#endif /* COLORTHEME_H */
