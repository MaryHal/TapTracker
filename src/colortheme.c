#include "colortheme.h"

#include <GLFW/glfw3.h>

float DARK_THEME[6][3] =
{
    { 0.97f, 0.97f, 0.97f }, // Single
    { 0.80f, 0.00f, 0.80f }, // Double
    { 0.00f, 0.80f, 0.80f }, // Triple
    { 0.90f, 0.90f, 0.00f }, // Tetris
    { 0.97f, 0.97f, 0.97f }, // Foreground
    { 0.05f, 0.05f, 0.05f }  // Background
};
float LIGHT_THEME[6][3] =
{
    { 0.12f, 0.15f, 0.17f }, // Single
    { 0.80f, 0.00f, 0.80f }, // Double
    { 0.00f, 0.80f, 0.80f }, // Triple
    { 0.90f, 0.90f, 0.00f }, // Tetris
    { 0.12f, 0.15f, 0.17f }, // Foreground
    { 0.95f, 0.96f, 0.96f }  // Background
};

// Set default theme
float (*colortheme)[6][3] = &DARK_THEME;

void setColorTheme(float (*theme)[6][3])
{
    colortheme = theme;
}

void setGLColor(unsigned int colorIndex, float alpha)
{
    glColor4f((*colortheme)[colorIndex][0],
              (*colortheme)[colorIndex][1],
              (*colortheme)[colorIndex][2],
              alpha);
}

void setGLClearColor()
{
    glClearColor((*colortheme)[COLOR_BACKGROUND][0],
                 (*colortheme)[COLOR_BACKGROUND][1],
                 (*colortheme)[COLOR_BACKGROUND][2],
                 0.0f);
}

void hexToRGB(float* rgbOutput, unsigned int hex)
{
    rgbOutput[0] = ((hex >> 16) & 0xFF) / 0xFF;
    rgbOutput[1] = ((hex >>  8) & 0xFF) / 0xFF;
    rgbOutput[2] = ((hex >>  0) & 0xFF) / 0xFF;
}
