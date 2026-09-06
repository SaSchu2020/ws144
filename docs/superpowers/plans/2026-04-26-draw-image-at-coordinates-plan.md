# Draw Image at Coordinates Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `drawBitmapAt` and `drawPngAt` functions to draw BMP/PNG images at specific X,Y coordinates on the 128x128 display.

**Architecture:** Refactor `GUI_ReadBmp` and `GUI_ReadPng` to accept X,Y offset parameters. Existing `drawBitmap`/`drawPng` delegate to new functions with (0,0). New JavaScript functions `drawBitmapAt`/`drawPngAt` accept path and coordinates.

**Tech Stack:** Node.js native addon (C++), node-gyp, lgpio

---

## File Structure

| File | Responsibility |
|------|----------------|
| `src/GUI/GUI_BMP.h` | Add `GUI_ReadBmp(path, x, y)` signature |
| `src/GUI/GUI_BMP.cc` | Implement offset-aware BMP drawing with clipping warning |
| `src/GUI/GUI_PNG.h` | Add `GUI_ReadPng(path, x, y)` signature |
| `src/GUI/GUI_PNG.cc` | Implement offset-aware PNG drawing with clipping warning |
| `src/display.h` | Add `drawBitmapAt`, `drawPngAt` declarations |
| `src/display.cc` | Implement new functions, refactor existing to use them |
| `src/addon.cc` | Export `drawBitmapAt`, `drawPngAt` functions |
| `test/test_binding.js` | Add tests for new functions |

---

### Task 1: Update GUI_BMP.h

**Files:**
- Modify: `src/GUI/GUI_BMP.h`

- [ ] **Step 1: Add new function signature**

```cpp
#ifndef GUI_BMP_H
#define GUI_BMP_H

#include "GUI_Paint.h"

UBYTE GUI_ReadBmp(const char *path);
UBYTE GUI_ReadBmp_WithOffset(const char *path, UWORD x, UWORD y);

#endif
```

- [ ] **Step 2: Commit**

```bash
git add src/GUI/GUI_BMP.h
git commit -m "feat: add GUI_ReadBmp_WithOffset signature"
```

---

### Task 2: Update GUI_BMP.cc

**Files:**
- Modify: `src/GUI/GUI_BMP.cc:24-60`

- [ ] **Step 1: Modify GUI_ReadBmp to delegate to GUI_ReadBmp_WithOffset**

Replace the existing `GUI_ReadBmp` function (lines 24-61) with:

```cpp
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
```

- [ ] **Step 2: Commit**

```bash
git add src/GUI/GUI_BMP.cc
git commit -m "feat: refactor GUI_ReadBmp to use offset-based GUI_ReadBmp_WithOffset"
```

---

### Task 3: Update GUI_PNG.h

**Files:**
- Modify: `src/GUI/GUI_PNG.h`

- [ ] **Step 1: Add new function signature**

```cpp
#ifndef GUI_PNG_H
#define GUI_PNG_H

#include "GUI_Paint.h"

UBYTE GUI_ReadPng(const char *path);
UBYTE GUI_ReadPng_WithOffset(const char *path, UWORD x, UWORD y);

#endif
```

- [ ] **Step 2: Commit**

```bash
git add src/GUI/GUI_PNG.h
git commit -m "feat: add GUI_ReadPng_WithOffset signature"
```

---

### Task 4: Update GUI_PNG.cc

**Files:**
- Modify: `src/GUI/GUI_PNG.cc:34-66`

- [ ] **Step 1: Modify GUI_ReadPng to delegate to GUI_ReadPng_WithOffset**

Replace the existing `GUI_ReadPng` function (lines 34-66) with:

```cpp
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
```

- [ ] **Step 2: Commit**

```bash
git add src/GUI/GUI_PNG.cc
git commit -m "feat: refactor GUI_ReadPng to use offset-based GUI_ReadPng_WithOffset"
```

---

### Task 5: Update display.h

**Files:**
- Modify: `src/display.h`

- [ ] **Step 1: Add new function declarations**

```cpp
#ifndef DISPLAY_H
#define DISPLAY_H

#include <napi.h>

Napi::Boolean writeText(const Napi::CallbackInfo& info);
Napi::Boolean drawBitmap(const Napi::CallbackInfo& info);
Napi::Boolean drawPng(const Napi::CallbackInfo& info);
Napi::Boolean setPixel(const Napi::CallbackInfo& info);
Napi::Boolean drawImageBuffer(const Napi::CallbackInfo& info);
Napi::Boolean clear(const Napi::CallbackInfo& info);
Napi::Boolean drawBitmapAt(const Napi::CallbackInfo& info);
Napi::Boolean drawPngAt(const Napi::CallbackInfo& info);

#endif
```

- [ ] **Step 2: Commit**

```bash
git add src/display.h
git commit -m "feat: add drawBitmapAt and drawPngAt declarations"
```

---

### Task 6: Update display.cc

**Files:**
- Modify: `src/display.cc`

- [ ] **Step 1: Update drawBitmap to use GUI_ReadBmp_WithOffset**

Change lines 32-43 from:
```cpp
Napi::Boolean drawBitmap(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    GUI_ReadBmp(path);

    LCD_1in44_Display(displayBuffer);

    return Napi::Boolean::New(env, true);
}
```

To:
```cpp
Napi::Boolean drawBitmap(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    GUI_ReadBmp_WithOffset(path, 0, 0);

    LCD_1in44_Display(displayBuffer);

    return Napi::Boolean::New(env, true);
}
```

- [ ] **Step 2: Update drawPng to use GUI_ReadPng_WithOffset**

Change lines 45-56 from:
```cpp
Napi::Boolean drawPng(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    GUI_ReadPng(path);

    LCD_1in44_Display(displayBuffer);

    return Napi::Boolean::New(env, true);
}
```

To:
```cpp
Napi::Boolean drawPng(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    GUI_ReadPng_WithOffset(path, 0, 0);

    LCD_1in44_Display(displayBuffer);

    return Napi::Boolean::New(env, true);
}
```

- [ ] **Step 3: Add new drawBitmapAt function after drawPng (after line 56)**

```cpp
Napi::Boolean drawBitmapAt(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    UWORD x = (uint32_t)info[1].As<Napi::Number>();
    UWORD y = (uint32_t)info[2].As<Napi::Number>();

    GUI_ReadBmp_WithOffset(path, x, y);

    LCD_1in44_Display(displayBuffer);

    return Napi::Boolean::New(env, true);
}

Napi::Boolean drawPngAt(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    UWORD x = (uint32_t)info[1].As<Napi::Number>();
    UWORD y = (uint32_t)info[2].As<Napi::Number>();

    GUI_ReadPng_WithOffset(path, x, y);

    LCD_1in44_Display(displayBuffer);

    return Napi::Boolean::New(env, true);
}
```

- [ ] **Step 4: Commit**

```bash
git add src/display.cc
git commit -m "feat: implement drawBitmapAt and drawPngAt with offset-based image drawing"
```

---

### Task 7: Update addon.cc

**Files:**
- Modify: `src/addon.cc`

- [ ] **Step 1: Add new exports to display object**

Add after line 24 (`display.Set(Napi::String::New(env, "drawPng"), Napi::Function::New(env, drawPng));`):

```cpp
display.Set(Napi::String::New(env, "drawBitmapAt"), Napi::Function::New(env, drawBitmapAt));
display.Set(Napi::String::New(env, "drawPngAt"), Napi::Function::New(env, drawPngAt));
```

- [ ] **Step 2: Commit**

```bash
git add src/addon.cc
git commit -m "feat: export drawBitmapAt and drawPngAt functions"
```

---

### Task 8: Build and Test

**Files:**
- Build: `npm run build`
- Test: `npm test` or `node test/test_binding.js`

- [ ] **Step 1: Build the native addon**

Run: `npm run build`
Expected: Build completes without errors

- [ ] **Step 2: Run existing tests**

Run: `npm test`
Expected: All existing tests pass (new code doesn't break existing functionality)

- [ ] **Step 3: Commit**

```bash
git add -A
git commit -m "test: verify build and existing tests pass"
```

---

## Self-Review Checklist

- [ ] Spec coverage: All requirements from spec have corresponding tasks
- [ ] Placeholder scan: No "TBD", "TODO", or incomplete steps
- [ ] Type consistency: Function signatures match across files (e.g., `GUI_ReadBmp_WithOffset` used consistently)
- [ ] No duplicated code: Existing `drawBitmap`/`drawPng` now delegate to new functions
