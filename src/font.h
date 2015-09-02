#ifndef FONT_H
#define FONT_H

#include <stdbool.h>
#include <stddef.h>

#include <uthash.h>
#include <stb_truetype.h>

struct chardata_t
{
        int id;
        UT_hash_handle hh;

        stbtt_packedchar pchar;
};

struct font_t
{
        unsigned int texture;
        unsigned int textureWidth;
        unsigned int textureHeight;
};

extern struct chardata_t* dataHash;

void addCharData(int codepoint, stbtt_packedchar pchar);
struct chardata_t* getCharData(int codepoint);
void deleteCharData();

struct font_t* loadFont(struct font_t* font, const char* filename, float pixelHeight);
void destroyFont(struct font_t* font, bool freeFont);

void getPackedQuad(struct font_t* font, int codepoint,
                   float* xpos, float* ypos,
                   int align_to_integer,
                   stbtt_aligned_quad* q);

void drawString(struct font_t* font, float x, float y, char* string);

#endif /* FONT_H */
