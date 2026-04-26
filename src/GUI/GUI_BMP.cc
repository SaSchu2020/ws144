/*****************************************************************************
* | File      	:   BMP_APP.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master 
*                and enhance portability
*----------------
* |	This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#include "GUI_BMP.h"
#include <stdio.h>	//fseek fread
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>	//memset

#include "GUI_Paint.h"
// #include "GUI_Cache.h"

UBYTE GUI_ReadBmp(const char *path)
{
    return GUI_ReadBmp_WithOffset(path, 0, 0);
}

UBYTE GUI_ReadBmp_WithOffset(const char *path, UWORD x, UWORD y)
{
    FILE *fp;
    BMPFILEHEADER bmpFileHeader;
    BMPINF bmpInfoHeader;

    if((fp = fopen(path, "rb")) == NULL) {
        DEBUG("Cann't open the file!\n");
        return 0;
    }

    fseek(fp, 0, SEEK_SET);
    fread(&bmpFileHeader, sizeof(BMPFILEHEADER), 1, fp);
    fread(&bmpInfoHeader, sizeof(BMPINF), 1, fp);

    unsigned long row, col;
    short data;
    RGBQUAD rgb;
    int len = bmpInfoHeader.bBitCount / 8;

    UWORD displayWidth = LCD_WIDTH;
    UWORD displayHeight = LCD_HEIGHT;
    UBYTE clipped = 0;

    fseek(fp, bmpFileHeader.bOffset, SEEK_SET);
    for(row = 0; row < bmpInfoHeader.bHeight; row++) {
        for(col = 0; col < bmpInfoHeader.bWidth; col++) {
            if(fread((char *)&rgb, 1, len, fp) != len){
                perror("get bmpdata:\r\n");
                break;
            }

            UWORD targetX = x + col;
            UWORD targetY = y + (bmpInfoHeader.bHeight - row - 1);

            if(targetX >= displayWidth || targetY >= displayHeight) {
                clipped = 1;
                continue;
            }

            data = RGB((rgb.rgbRed), (rgb.rgbGreen), (rgb.rgbBlue));
            Paint_SetPixel(targetX, targetY, data);
        }
    }
    fclose(fp);

    if(clipped) {
        DEBUG("Warning: image extends beyond display bounds, clipping\n");
    }

    return 0;
}