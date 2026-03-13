#include "GUI_PNG.h"
#include "GUI_BMP.h"
#include "spng.h"
#include <stdio.h>
#include <stdlib.h>

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
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        DEBUG("Can't open PNG file: %s\n", path);
        return 1;
    }
    
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    unsigned char *fileBuffer = (unsigned char *)malloc(fileSize);
    if (fileBuffer == NULL) {
        DEBUG("Failed to allocate memory\n");
        fclose(fp);
        return 1;
    }
    fread(fileBuffer, 1, fileSize, fp);
    fclose(fp);
    
    spng_ctx *ctx = spng_ctx_new(0);
    if (ctx == NULL) {
        DEBUG("Failed to create spng context\n");
        free(fileBuffer);
        return 1;
    }
    
    int ret = spng_set_png_buffer(ctx, fileBuffer, fileSize);
    if (ret) {
        DEBUG("spng_set_png_buffer error: %s\n", spng_strerror(ret));
        free(fileBuffer);
        spng_ctx_free(ctx);
        return 1;
    }
    
    spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);
    if (ret) {
        DEBUG("spng_get_ihdr error: %s\n", spng_strerror(ret));
        free(fileBuffer);
        spng_ctx_free(ctx);
        return 1;
    }
    
    size_t imageSize;
    ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &imageSize);
    if (ret) {
        DEBUG("spng_decoded_image_size error: %s\n", spng_strerror(ret));
        free(fileBuffer);
        spng_ctx_free(ctx);
        return 1;
    }
    
    unsigned char *imageBuffer = (unsigned char *)malloc(imageSize);
    if (imageBuffer == NULL) {
        DEBUG("Failed to allocate image buffer\n");
        free(fileBuffer);
        spng_ctx_free(ctx);
        return 1;
    }
    
    ret = spng_decode_image(ctx, imageBuffer, imageSize, SPNG_FMT_RGBA8, SPNG_DECODE_TRNS);
    if (ret) {
        DEBUG("spng_decode_image error: %s\n", spng_strerror(ret));
        free(imageBuffer);
        free(fileBuffer);
        spng_ctx_free(ctx);
        return 1;
    }
    
    for (UWORD row = 0; row < ihdr.height; row++) {
        for (UWORD col = 0; col < ihdr.width; col++) {
            size_t idx = (row * ihdr.width + col) * 4;
            
            UBYTE R = imageBuffer[idx];
            UBYTE G = imageBuffer[idx + 1];
            UBYTE B = imageBuffer[idx + 2];
            UBYTE A = imageBuffer[idx + 3];
            
            if (col >= Paint.Width || row >= Paint.Height) continue;
            
            if (A < 255) {
                UWORD existing = Paint_GetPixel(col, row);
                Paint_SetPixel(col, row, GUI_BlendPixel(existing, R, G, B, A));
            } else {
                Paint_SetPixel(col, row, RGB(R, G, B));
            }
        }
    }
    
    free(imageBuffer);
    free(fileBuffer);
    spng_ctx_free(ctx);
    return 0;
}
