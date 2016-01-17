#include "font.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <assert.h>

#include <GLFW/glfw3.h>

#include <stb_rect_pack.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

struct chardata_t* _getCharData(struct chardata_t** cmap, int codepoint)
{
    struct chardata_t* s = NULL;

    HASH_FIND_INT(*cmap, &codepoint, s);  /* s: output */
    return s;
}

void _addCharData(struct chardata_t** cmap, int codepoint, stbtt_packedchar pchar)
{
    struct chardata_t* s = _getCharData(cmap, codepoint);

    if (s == NULL)
    {
        s = malloc(sizeof(struct chardata_t));
        s->id = codepoint;
        s->pchar = pchar;

        HASH_ADD_INT(*cmap, id, s);  /* id: name of key field */
    }
}

/* void deleteCharData(struct chardata_t** cmap, struct chardata_t* cdata) */
/* { */
/*     HASH_DEL(*cmap, cdata);  /\* user: pointer to deletee *\/ */
/*     free(cdata); */
/* } */

void exportBitmap(const char* imgOutFilename, struct font_t* font)
{
    assert(imgOutFilename != NULL && font != NULL);

    if (font->bitmap == NULL)
    {
        fprintf(stderr, "Cannot export font bitmap. Is this font already a bitmap font?");
    }

    stbi_write_png(imgOutFilename, font->textureWidth, font->textureHeight, 1, font->bitmap, 0);
}

void exportFontData(const char* binOutFilename, struct font_t* font)
{
    assert(binOutFilename != NULL && font != NULL);

    FILE* binOutput = fopen(binOutFilename, "wb");

    if (!binOutput)
    {
        perror("Could not open file for font data output");
        return;
    }

    fwrite(font, sizeof(struct font_t), 1, binOutput);

    for (struct chardata_t* cdata = font->cmap; cdata != NULL; cdata = cdata->hh.next)
    {
        fwrite(&cdata->id, sizeof(int), 1, binOutput);
        fwrite(&cdata->pchar, sizeof(stbtt_packedchar), 1, binOutput);
    }

    fclose(binOutput);
}

void flushFontBitmap(struct font_t* font)
{
    assert(font != NULL);

    if (font->bitmap)
    {
        free(font->bitmap);
        font->bitmap = NULL;
    }
}

void _loadTTF_file(const char* filename, uint8_t** ttfData)
{
    assert(filename != NULL && ttfData != NULL);

    FILE* ttf_file = fopen(filename, "rb");

    if (!ttf_file)
    {
        perror("Could not open TTF file");
        return;
    }

    // Get filesize so we know how much memory to allocate.
    fseek(ttf_file, 0L, SEEK_END);
    size_t filesize = ftell(ttf_file);
    rewind(ttf_file);

    *ttfData = malloc(sizeof(uint8_t) * filesize);
    size_t readsize = fread(*ttfData, 1, filesize, ttf_file);

    assert(filesize == readsize);

    fclose(ttf_file);
}

void _bindFontTexture(struct font_t* font, uint8_t* bitmap)
{
    assert(font != NULL);

    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                 font->textureWidth, font->textureHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);

    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

struct font_t* loadTTF(struct font_t* font, const char* filename, float pixelHeight)
{
    if (font == NULL)
    {
        font = malloc(sizeof(struct font_t));
    }

    font->cmap = NULL;
    font->texture = 0;

    font->textureWidth  = 512;
    font->textureHeight = 512;
    font->pixelHeight = pixelHeight;

    font->bitmap = NULL;

    uint8_t* ttf_buffer = NULL;
    _loadTTF_file(filename, &ttf_buffer);
    {
        font->bitmap = malloc(sizeof(uint8_t) * font->textureWidth * font->textureHeight);

        // Pack our font
        stbtt_pack_context pc;

        if (!stbtt_PackBegin(&pc, font->bitmap, font->textureWidth, font->textureHeight, 0, 1, NULL))
        {
            perror("stbtt_PackBegin error");
        }

        const int NUM_RANGES = 2;
        stbtt_pack_range pr[NUM_RANGES];
        stbtt_packedchar pdata[256*2];

        pr[0].chardata_for_range = pdata;
        pr[0].first_unicode_codepoint_in_range = 32;
        pr[0].num_chars = 95;
        pr[0].font_size = pixelHeight;

        pr[1].chardata_for_range = pdata+256;
        pr[1].first_unicode_codepoint_in_range = 0x2190;
        pr[1].num_chars = 0x2193 - 0x2190 + 1;
        pr[1].font_size = pixelHeight;

        stbtt_PackSetOversampling(&pc, 2, 2);

        if (!stbtt_PackFontRanges(&pc, ttf_buffer, 0, pr, 2))
        {
            perror("stbtt_PackFontRanges error. Chars cannot fit on bitmap?");
            return NULL;
        }

        stbtt_PackEnd(&pc);

        // Move all rects to hash table.
        for (int i = 0; i < NUM_RANGES; i++)
        {
            for (int j = 0; j < pr[i].num_chars; j++)
            {
                int codepoint = pr[i].first_unicode_codepoint_in_range + j;
                /* int codepoint = j; */

                _addCharData(&font->cmap, codepoint, pdata[i * 256 + j]);
            }
        }

        _bindFontTexture(font, font->bitmap);
    }
    free(ttf_buffer);

    return font;
}

struct font_t* loadBitmapFontFiles(struct font_t* font, const char* imgFile, const char* binFile)
{
    if (font == NULL)
    {
        font = malloc(sizeof(struct font_t));
    }

    FILE* binInput = fopen(binFile, "rb");

    if (!binInput)
    {
        perror("Could not open file for font data output");
        return NULL;
    }

    fread(font, sizeof(struct font_t), 1, binInput);

    font->cmap = NULL;
    font->bitmap = NULL;

    // Load char data.
    int codepoint = 0;
    stbtt_packedchar pchar;

    while (fread(&codepoint, sizeof(int), 1, binInput) == 1 &&
           fread(&pchar, sizeof(stbtt_packedchar), 1, binInput) == 1)
    {
        _addCharData(&font->cmap, codepoint, pchar);
    }

    fclose(binInput);

    // Load bitmap and set it up for OpenGL to use.
    int x, y, n;
    uint8_t* temp_bitmap = stbi_load(imgFile, &x, &y, &n, 0);

    assert(x == font->textureWidth);
    assert(y == font->textureHeight);

    _bindFontTexture(font, temp_bitmap);

    stbi_image_free(temp_bitmap);

    return font;
}

struct font_t* loadBitmapFontData(struct font_t* font,
                                  const uint8_t imgData[], size_t imgDataSize,
                                  const uint8_t binData[], size_t binDataSize)
{
    if (font == NULL)
    {
        font = malloc(sizeof(struct font_t));
    }

    const uint8_t* dataPtr = binData;

    // Load font struct
    *font = *(struct font_t*)(dataPtr);
    dataPtr += sizeof(struct font_t);
    binDataSize -= sizeof(struct font_t);

    font->cmap = NULL;
    font->bitmap = NULL;

    while (binDataSize > 0)
    {
        int codepoint = *(int*)(dataPtr);
        dataPtr += sizeof(int);
        binDataSize -= sizeof(int);

        stbtt_packedchar pchar = *(stbtt_packedchar*)(dataPtr);
        dataPtr += sizeof(stbtt_packedchar);
        binDataSize -= sizeof(stbtt_packedchar);

        _addCharData(&font->cmap, codepoint, pchar);
    }

    // Load bitmap and set it up for OpenGL to use.
    int x, y, n;
    uint8_t* temp_bitmap = stbi_load_from_memory(imgData, imgDataSize, &x, &y, &n, 0);

    assert(x == font->textureWidth);
    assert(y == font->textureHeight);

    _bindFontTexture(font, temp_bitmap);

    stbi_image_free(temp_bitmap);

    return font;
}

void destroyFont(struct font_t* font, bool freeFont)
{
    glDeleteTextures(1, &font->texture);

    struct chardata_t* current_cdata = NULL;
    struct chardata_t* tmp = NULL;

    HASH_ITER(hh, font->cmap, current_cdata, tmp)
    {
        HASH_DEL(font->cmap, current_cdata);
        free(current_cdata);
    }

    if (font->bitmap)
        free(font->bitmap);

    if (freeFont)
        free(font);
}

void getPackedQuad(struct font_t* font, int codepoint,
                   float* xpos, float* ypos,
                   int align_to_integer,
                   stbtt_aligned_quad* q)
{
    float ipw = 1.0f / font->textureWidth;
    float iph = 1.0f / font->textureHeight;

    assert(_getCharData(&font->cmap, codepoint) != NULL);

    stbtt_packedchar* b = &_getCharData(&font->cmap, codepoint)->pchar;

    if (align_to_integer)
    {
        float x = (float) ((int)floor((*xpos + b->xoff) + 0.5f));
        float y = (float) ((int)floor((*ypos + b->yoff) + 0.5f));
        q->x0 = x;
        q->y0 = y;
        q->x1 = x + b->xoff2 - b->xoff;
        q->y1 = y + b->yoff2 - b->yoff;
    }
    else
    {
        q->x0 = *xpos + b->xoff;
        q->y0 = *ypos + b->yoff;
        q->x1 = *xpos + b->xoff2;
        q->y1 = *ypos + b->yoff2;
    }

    q->s0 = b->x0 * ipw;
    q->t0 = b->y0 * iph;
    q->s1 = b->x1 * ipw;
    q->t1 = b->y1 * iph;

    *xpos += b->xadvance;
}

void drawString(struct font_t* font, float x, float y, const char* string)
{
    size_t len = strlen(string);
    float vertices[len * 8];
    float texCoords[len * 8];

    for (size_t i = 0; i < len; ++i)
    {
        stbtt_aligned_quad q;
        getPackedQuad(font, string[i], &x, &y, 0, &q);

        vertices[i * 8 + 0] = q.x0; vertices[i * 8 + 1] = q.y0;
        vertices[i * 8 + 2] = q.x1; vertices[i * 8 + 3] = q.y0;
        vertices[i * 8 + 4] = q.x1; vertices[i * 8 + 5] = q.y1;
        vertices[i * 8 + 6] = q.x0; vertices[i * 8 + 7] = q.y1;

        texCoords[i * 8 + 0] = q.s0; texCoords[i * 8 + 1] = q.t0;
        texCoords[i * 8 + 2] = q.s1; texCoords[i * 8 + 3] = q.t0;
        texCoords[i * 8 + 4] = q.s1; texCoords[i * 8 + 5] = q.t1;
        texCoords[i * 8 + 6] = q.s0; texCoords[i * 8 + 7] = q.t1;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    glDrawArrays(GL_QUADS, 0, len * 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_TEXTURE_2D);
}

float getStringWidth(struct font_t* font, const char* string)
{
    float x = 0;
    float y = 0;

    for (;*string != '\0'; ++string)
    {
        stbtt_aligned_quad q;
        getPackedQuad(font, *string, &x, &y, 0, &q);
    }

    return x;
}
