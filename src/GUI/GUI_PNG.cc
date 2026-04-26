#include "GUI_PNG.h"
#include "GUI_BMP.h"
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void GUI_ExtractRGB565(UWORD color, UBYTE *R, UBYTE *G, UBYTE *B)
{
    *R = (color >> 11) & 0x1F;
    *G = (color >> 5) & 0x3F;
    *B = color & 0x1F;
    *R = (*R << 3) | (*R >> 2);
    *G = (*G << 2) | (*G >> 4);
    *B = (*B << 3) | (*B >> 2);
}

static UWORD GUI_BlendPixel(UWORD existingColor, UBYTE R, UBYTE G, UBYTE B, UBYTE A)
{
    if (A == 0) return existingColor;
    if (A == 255) return RGB(R, G, B);
    
    UBYTE existR, existG, existB;
    GUI_ExtractRGB565(existingColor, &existR, &existG, &existB);
    
    UBYTE blendR = (R * A + existR * (255 - A)) >> 8;
    UBYTE blendG = (G * A + existG * (255 - A)) >> 8;
    UBYTE blendB = (B * A + existB * (255 - A)) >> 8;
    
    return RGB(blendR, blendG, blendB);
}

UBYTE GUI_ReadPng(const char *path)
{
    return GUI_ReadPng_WithOffset(path, 0, 0);
}

UBYTE GUI_ReadPng_WithOffset(const char *path, UWORD x, UWORD y)
{
    int width, height, channels;
    unsigned char *img = stbi_load(path, &width, &height, &channels, 4);

    if (img == NULL) {
        DEBUG("Failed to load PNG: %s\n", stbi_failure_reason());
        return 1;
    }

    UWORD displayWidth = LCD_WIDTH;
    UWORD displayHeight = LCD_HEIGHT;
    UBYTE clipped = 0;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int idx = (row * width + col) * 4;

            UBYTE R = img[idx];
            UBYTE G = img[idx + 1];
            UBYTE B = img[idx + 2];
            UBYTE A = img[idx + 3];

            UWORD targetX = x + col;
            UWORD targetY = y + row;

            if (targetX >= displayWidth || targetY >= displayHeight) {
                clipped = 1;
                continue;
            }

            if (A < 255) {
                UWORD existing = Paint_GetPixel(targetX, targetY);
                Paint_SetPixel(targetX, targetY, GUI_BlendPixel(existing, R, G, B, A));
            } else {
                Paint_SetPixel(targetX, targetY, RGB(R, G, B));
            }
        }
    }

    stbi_image_free(img);

    if(clipped) {
        DEBUG("Warning: image extends beyond display bounds, clipping\n");
    }

    return 0;
}
