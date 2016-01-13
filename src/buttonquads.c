#include "buttonquads.h"

#include <stdint.h>
#include <stdlib.h>

#include <stb_image.h>
#include <GLFW/glfw3.h>

#define INCBIN_PREFIX g_
#include <incbin.h>
INCBIN(ButtonSheet, "bin/key_button.png");

struct button_spectrum_t* createButtonSheet(struct button_spectrum_t* bspec)
{
    if (!bspec)
    {
        bspec = (struct button_spectrum_t*)malloc(sizeof(struct button_spectrum_t));
    }

    const int tileSize = 8;

    int width, height, n;
    uint8_t* bitmap = stbi_load_from_memory(g_ButtonSheetData, g_ButtonSheetSize, &width, &height, &n, 4);

    glGenTextures(1, &bspec->textureID);
    glBindTexture(GL_TEXTURE_2D, bspec->textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(bitmap);

    // Setup coordinates
    for (size_t i = 0; i < BUTTON_INDEX_COUNT; ++i)
    {
        struct button_spectrum_quad_t* currentQuad = &bspec->quads[i];

        // This also accounts for padding in our texture atlas (since we're
        // using GL_LINEAR). Without the padding pixels, when we get some
        // texture bleeding.
        currentQuad->texCoords[0] = (float)(i * tileSize + i) / width;
        currentQuad->texCoords[1] = (float)0.0f;
        currentQuad->texCoords[2] = (float)(i * tileSize + i + tileSize) / width;
        currentQuad->texCoords[3] = (float)0.0f;
        currentQuad->texCoords[4] = (float)(i * tileSize + i + tileSize) / width;
        currentQuad->texCoords[5] = (float)1.0f;
        currentQuad->texCoords[6] = (float)(i * tileSize + i) / width;
        currentQuad->texCoords[7] = (float)1.0f;
    }

    return bspec;
}

void destroyButtonSheet(struct button_spectrum_t* bspec, bool freeMe)
{
    glDeleteTextures(1, &bspec->textureID);

    if (freeMe)
        free(bspec);
}
