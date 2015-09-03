#include "font.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <assert.h>

#include <GLFW/glfw3.h>

#include <stb_rect_pack.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

struct chardata_t* dataHash = NULL;

void addCharData(int codepoint, stbtt_packedchar pchar)
{
    struct chardata_t* s = getCharData(codepoint);

    if (s == NULL)
    {
        s = (struct chardata_t*)malloc(sizeof(struct chardata_t));
        s->id = codepoint;
        s->pchar = pchar;

        HASH_ADD_INT(dataHash, id, s);  /* id: name of key field */
    }
}

struct chardata_t* getCharData(int codepoint)
{
    struct chardata_t* s = NULL;

    HASH_FIND_INT(dataHash, &codepoint, s);  /* s: output */
    return s;
}

void deleteCharData(struct chardata_t* cdata)
{
    HASH_DEL(dataHash, cdata);  /* user: pointer to deletee */
    free(cdata);
}

struct font_t* loadFont(struct font_t* font, const char* filename, float pixelHeight)
{
    if (font == NULL)
    {
        font = (struct font_t*)malloc(sizeof(struct font_t));
    }
    font->textureWidth  = 512;
    font->textureHeight = 512;

    /* font->dataHash = NULL; */

    FILE* ttf_file = fopen(filename, "rb");

    // Get filesize so we know how much memory to allocate.
    fseek(ttf_file, 0L, SEEK_END);
    size_t filesize = ftell(ttf_file);
    rewind(ttf_file);

    uint8_t ttf_buffer[filesize];
    uint8_t temp_bitmap[font->textureWidth * font->textureHeight];

    fread(ttf_buffer, 1, filesize, fopen(filename, "rb"));

    fclose(ttf_file);

    // Pack our font
    {
        stbtt_pack_context pc;

        if (!stbtt_PackBegin(&pc, temp_bitmap, font->textureWidth, font->textureHeight, 0, 1, NULL))
        {
            perror("stbtt_PackBegin error");
        }

        stbtt_packedchar pdata[256*2];
        stbtt_pack_range pr[2];

        pr[0].chardata_for_range = pdata;
        pr[0].first_unicode_codepoint_in_range = 32;
        pr[0].num_chars = 95;
        pr[0].font_size = STBTT_POINT_SIZE(pixelHeight);

        pr[1].chardata_for_range = pdata+256;
        pr[1].first_unicode_codepoint_in_range = 0x21d0;
        pr[1].num_chars = 0x21d3 - 0x2100 + 1;
        pr[1].font_size = STBTT_POINT_SIZE(pixelHeight);

        stbtt_PackSetOversampling(&pc, 2, 2);

        if (!stbtt_PackFontRanges(&pc, ttf_buffer, 0, pr, 2))
        {
            perror("stbtt_PackFontRanges error. Chars cannot fit on bitmap?");
        }

        stbtt_PackEnd(&pc);
        stbi_write_png("fonttest3.png", font->textureWidth, font->textureHeight, 1, temp_bitmap, 0);

        // Move all rects to hash table.
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < pr[i].num_chars; j++)
            {
                int codepoint = pr[i].first_unicode_codepoint_in_range + j;
                /* int codepoint = j; */

                addCharData(codepoint, pdata[i * 256 + j]);
            }
        }
    }

    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                 font->textureWidth, font->textureHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);

    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return font;
}

void destroyFont(struct font_t* font, bool freeFont)
{
    glDeleteTextures(1, &font->texture);

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

    assert(getCharData(codepoint) != NULL);

    stbtt_packedchar* b = &getCharData(codepoint)->pchar;

    if (align_to_integer) {
        float x = (float) ((int)floor((*xpos + b->xoff) + 0.5f));
        float y = (float) ((int)floor((*ypos + b->yoff) + 0.5f));
        q->x0 = x;
        q->y0 = y;
        q->x1 = x + b->xoff2 - b->xoff;
        q->y1 = y + b->yoff2 - b->yoff;
    } else {
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

void drawChar(struct font_t* font, float* x, float* y, wchar_t c)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    glBegin(GL_QUADS);
    {
        stbtt_aligned_quad q;
        getPackedQuad(font, c, x, y, 1, &q);

        glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
        glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
        glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
        glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawString(struct font_t* font, float x, float y, const char* string)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    // Boo! Immediate mode!
    for (;*string != '\0'; ++string)
    {
        drawChar(font, &x, &y, *string);
    }

    glDisable(GL_TEXTURE_2D);
}

void drawWideString(struct font_t* font, float x, float y, const wchar_t* string)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    // Boo! Immediate mode!
    for (;*string != '\0'; ++string)
    {
        drawChar(font, &x, &y, *string);
    }

    glDisable(GL_TEXTURE_2D);
}
