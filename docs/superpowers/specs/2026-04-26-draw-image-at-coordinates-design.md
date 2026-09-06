# Design: Draw Image at Coordinates

**Date**: 2026-04-26
**Feature**: Add `drawBitmapAt` and `drawPngAt` functions to draw images at specific X,Y coordinates

## Overview

Add the ability to draw smaller BMP/PNG images at specific coordinates on the 128x128 display, not just full-screen at (0,0).

## Architecture

- **New JS functions**: `Display.drawBitmapAt(path, x, y)` and `Display.drawPngAt(path, x, y)`
- **Refactored internal loaders**: `GUI_ReadBmp()` and `GUI_ReadPng()` become internal helpers taking x,y offsets
- **Existing functions** (`drawBitmap`, `drawPng`) call the new loaders with (0, 0)

### Data Flow

```
JavaScript: drawBitmapAt(path, 20, 30)
    → display.cc: drawBitmapAt()
        → GUI_BMP.cc: GUI_ReadBmp(path, x=20, y=30)
            → Paint_SetPixel(x + 20, y + 30, color)  // with bounds check
                → displayBuffer (128x128)
```

## Function Signatures

### New Functions

```c
// In GUI_BMP.h / GUI_PNG.h
UBYTE GUI_ReadBmp(const char *path, UWORD x, UWORD y);
UBYTE GUI_ReadPng(const char *path, UWORD x, UWORD y);

// In display.h
Napi::Boolean drawBitmapAt(const Napi::CallbackInfo& info);
Napi::Boolean drawPngAt(const Napi::CallbackInfo& info);
```

### Modified Functions

```c
// Existing functions now delegate to new implementations with (0, 0)
Napi::Boolean drawBitmap(const Napi::CallbackInfo& info) {
    // calls GUI_ReadBmp(path, 0, 0)
}

Napi::Boolean drawPng(const Napi::CallbackInfo& info) {
    // calls GUI_ReadPng(path, 0, 0)
}
```

## Boundary Handling

- If an image pixel maps to a display coordinate >= 128, skip that pixel (clip)
- When clipping occurs, print once to console: `"Warning: image extends beyond display bounds, clipping"`
- No error returned - clipping is treated as a non-fatal condition

## JavaScript API

```javascript
Display.drawBitmapAt(path, x, y)  // Draw BMP at position (x, y)
Display.drawPngAt(path, x, y)     // Draw PNG at position (x, y)
Display.drawBitmap(path)           // Unchanged - draws at (0, 0)
Display.drawPng(path)             // Unchanged - draws at (0, 0)
```

## Files to Modify

| File | Change |
|------|--------|
| `src/GUI/GUI_BMP.h` | Add `GUI_ReadBmp(path, x, y)` signature |
| `src/GUI/GUI_BMP.cc` | Implement offset-aware drawing with clipping warning |
| `src/GUI/GUI_PNG.h` | Add `GUI_ReadPng(path, x, y)` signature |
| `src/GUI/GUI_PNG.cc` | Implement offset-aware drawing with clipping warning |
| `src/display.h` | Add `drawBitmapAt`, `drawPngAt` declarations |
| `src/display.cc` | Implement new functions, refactor existing to use them |
| `src/addon.cc` | Export `drawBitmapAt`, `drawPngAt` functions |

## Testing

1. Draw 50x50 image at (0, 0) - should appear in top-left corner
2. Draw 50x50 image at (78, 78) - should appear in bottom-right corner
3. Draw 100x100 image at (50, 50) - should clip, console warning appears
4. Existing `drawBitmap`/`drawPng` still work at (0, 0)

## Constraints

- Display size: 128x128 pixels (LCD_HEIGHT, LCD_WIDTH)
- Maximum image dimensions: 128x128
- Supported formats: BMP (RGB), PNG (with alpha blending)
