#ifndef FONT_H
#define FONT_H

#include <stdbool.h>
#include <stddef.h>

#include <stb_truetype.h>

struct font_t
{
        unsigned int texture;
        unsigned int textureWidth;
        unsigned int textureHeight;
        stbtt_packedchar pdata[256*2];
};

struct font_t* loadFont(struct font_t* font, const char* filename, float pixelHeight);
void destroyFont(struct font_t* font, bool freeFont);

void drawText(struct font_t* font, float x, float y, char* string);

#endif /* FONT_H */
