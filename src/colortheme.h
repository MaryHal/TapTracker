#ifndef COLORTHEME_H
#define COLORTHEME_H

enum COLOR_INDEX
{
    COLOR_SINGLE = 0,
    COLOR_DOUBLE,
    COLOR_TRIPLE,
    COLOR_TETRIS,
    COLOR_FOREGROUND,
    COLOR_BACKGROUND,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    NUM_COLORS
};

extern float DARK_THEME[NUM_COLORS][3];
extern float LIGHT_THEME[NUM_COLORS][3];

extern float (*colortheme)[NUM_COLORS][3];

const float* getColorFromTheme(unsigned int colorIndex);
void setColorTheme(float (*theme)[NUM_COLORS][3]);
void setGLColor(unsigned int colorIndex, float alpha);
void setGLClearColor();

void hexToRGB(float* rgbOutput, unsigned int hex);

#endif /* COLORTHEME_H */
