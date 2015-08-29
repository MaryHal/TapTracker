#include "font.h"

#include <stdlib.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

#include <stb_rect_pack.h>

/* #define STB_IMAGE_WRITE_IMPLEMENTATION */
/* #include <stb_image_write.h> */

struct font_t* loadFont(struct font_t* font, const char* filename, float pixelHeight)
{
    if (font == NULL)
    {
        font = (struct font_t*)malloc(sizeof(struct font_t));
    }
    font->textureWidth  = 512;
    font->textureHeight = 512;

    FILE* ttf_file = fopen(filename, "rb");

    // Get filesize so we know how much memory to allocate.
    fseek(ttf_file, 0L, SEEK_END);
    size_t filesize = ftell(ttf_file);
    rewind(ttf_file);

    uint8_t ttf_buffer[filesize];
    uint8_t temp_bitmap[font->textureWidth * font->textureHeight];

    fread(ttf_buffer, 1, 1 << 20, fopen(filename, "rb"));

    fclose(ttf_file);

    // Pack our font
    {
        stbtt_pack_context pc;

        if (!stbtt_PackBegin(&pc, temp_bitmap, font->textureWidth, font->textureHeight, 0, 1, NULL))
        {
            perror("stbtt_PackBegin error");
        }

        stbtt_pack_range pr[2];

        pr[0].chardata_for_range = font->pdata;
        pr[0].first_unicode_codepoint_in_range = 32;
        pr[0].num_chars = 95;
        pr[0].font_size = pixelHeight;

        pr[1].chardata_for_range = font->pdata+256;
        pr[1].first_unicode_codepoint_in_range = 0xa0;
        pr[1].num_chars = 0x100 - 0xa0;
        pr[1].font_size = pixelHeight;

        stbtt_PackSetOversampling(&pc, 2, 2);

        if (!stbtt_PackFontRanges(&pc, ttf_buffer, 0, pr, 2))
        {
            perror("stbtt_PackFontRanges error. Chars cannot fit on bitmap?");
        }

        stbtt_PackEnd(&pc);
        /* stbi_write_png("fonttest3.png", font->textureWidth, font->textureHeight, 1, temp_bitmap, 0); */
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

void drawText(struct font_t* font, float x, float y, char* string)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    // Boo! Immediate mode!
    for (;*string != '\0'; ++string)
    {
        glBegin(GL_QUADS);
        {
            stbtt_aligned_quad q;
            stbtt_GetPackedQuad(font->pdata,
                                font->textureWidth, font->textureHeight,
                                *string - ' ',
                                &x, &y, &q,
                                1); //1=opengl & d3d10+,0=d3d9

            glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
            glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
            glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
            glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}
