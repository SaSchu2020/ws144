#ifndef __GUI_PNG_H
#define __GUI_PNG_H

#include <stdint.h>
#include "GUI_Paint.h"

UBYTE GUI_ReadPng(const char *path);
UBYTE GUI_ReadPng_WithOffset(const char *path, UWORD x, UWORD y);

#endif
