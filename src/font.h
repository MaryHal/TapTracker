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
        int textureWidth;
        int textureHeight;

        float pixelHeight;

        uint8_t* bitmap;
};

extern struct chardata_t* dataHash;

// uthash functions
void addCharData(int codepoint, stbtt_packedchar pchar);
struct chardata_t* getCharData(int codepoint);
void deleteCharData();

// Export internal bitmap for a font. This should not be used if this font is
// already a bitmap font.
void exportBitmap(const char* imgFile, struct font_t* font);
void exportFontData(const char* binFile, struct font_t* font);

// Loads a TTF file on the heap into *ttfData. Remember to free it!
void _loadTTF_file(const char* filename, uint8_t** ttfData);

// Load a bitmap into a font's texture handle.
void _bindFontTexture(struct font_t* font, uint8_t* bitmap);

struct font_t* _createFont(struct font_t* font);

struct font_t* loadTTF(struct font_t* font, const char* filename, float pixelHeight);
struct font_t* loadBitmapFont(struct font_t* font, const char* imgFile, const char* binFile);
void destroyFont(struct font_t* font, bool freeFont);

void getPackedQuad(struct font_t* font, int codepoint,
                   float* xpos, float* ypos,
                   int align_to_integer,
                   stbtt_aligned_quad* q);

void drawChar(struct font_t* font, float* x, float* y, wchar_t c);
void drawString(struct font_t* font, float x, float y, const char* string);
void drawWideString(struct font_t* font, float x, float y, const wchar_t* string);

float getStringWidth(struct font_t* font, const char* string);

#endif /* FONT_H */
