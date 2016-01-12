#include "inputhistory.h"

#include <stdint.h>
#include <stdlib.h>

#include <stb_image.h>
#include <GLFW/glfw3.h>

#include <incbin.h>

INCBIN(ButtonSheet, "src/key_button.png");

struct button_spectrum_t* createButtonSheet(struct button_spectrum_t* bspec)
{
    if (!bspec)
    {
        bspec = (struct button_spectrum_t*)malloc(sizeof(struct button_spectrum_t));
    }

    int width, height, n;
    uint8_t* bitmap = stbi_load_from_memory(gButtonSheetData, gButtonSheetSize, &width, &height, &n, 4);

    glGenTextures(1, &bspec->textureID);
    glBindTexture(GL_TEXTURE_2D, bspec->textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

    // can free bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Setup coordinates
    for (size_t i = 0; i < INPUT_COUNT; ++i)
    {
        struct button_spectrum_quad_t* currentQuad = &bspec->quads[i];

        currentQuad->texCoords[0] = (i * 8.0f) / width;
        currentQuad->texCoords[1] = 0.0f;
        currentQuad->texCoords[2] = (i * 8.0f + 8.0f) / width;
        currentQuad->texCoords[3] = 0.0f;
        currentQuad->texCoords[4] = (i * 8.0f + 8.0f) / width;
        currentQuad->texCoords[5] = 1.0f;
        currentQuad->texCoords[6] = (i * 8.0f) / width;
        currentQuad->texCoords[7] = 1.0f;
    }

    return bspec;
}

void destroyButtonSheet(struct button_spectrum_t* bspec, bool freeMe)
{
    glDeleteTextures(1, &bspec->textureID);

    if (freeMe)
        free(bspec);
}

