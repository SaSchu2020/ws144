#include <napi.h>

#include "LCD_1in44.h"
#include "fonts.h"
#include "GUI_BMP.h"
#include "GUI_PNG.h"
#include "GUI_Paint.h"

#include "addon.h"

#include "display.h"

// Deferred-flush state for beginDraw()/endDraw().
// When buffering is active, drawing operations write to displayBuffer but
// skip the per-operation hardware flush. endDraw() performs the single flush.
static bool g_buffering = false;

// Flush the buffer to the physical LCD only when not in a buffered session.
static void PresentIfLive() {
    if (!g_buffering) {
        LCD_1in44_Display(displayBuffer);
    }
}

Napi::Boolean writeText(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string textStr = info[0].As<Napi::String>().Utf8Value();
    const char* text = textStr.c_str();

    UWORD x = (uint32_t)info[1].As<Napi::Number>();
    UWORD y = (uint32_t)info[2].As<Napi::Number>();

    UWORD color = (uint32_t)info[3].As<Napi::Number>();
    UWORD backgroundColor = (uint32_t)info[4].As<Napi::Number>();

    Paint_DrawString_EN(x, y, text, &Font12, color, backgroundColor);

    PresentIfLive();

	return Napi::Boolean::New(env, true);
}

Napi::Boolean drawBitmap(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

GUI_ReadBmp_WithOffset(path, 0, 0);

    PresentIfLive();

	return Napi::Boolean::New(env, true);
}

Napi::Boolean drawPng(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    GUI_ReadPng_WithOffset(path, 0, 0);

    PresentIfLive();

	return Napi::Boolean::New(env, true);
}

Napi::Boolean drawBitmapAt(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    UWORD x = (uint32_t)info[1].As<Napi::Number>();
    UWORD y = (uint32_t)info[2].As<Napi::Number>();

    GUI_ReadBmp_WithOffset(path, x, y);

    PresentIfLive();

    return Napi::Boolean::New(env, true);
}

Napi::Boolean drawPngAt(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string pathStr = info[0].As<Napi::String>().Utf8Value();
    const char* path = pathStr.c_str();

    UWORD x = (uint32_t)info[1].As<Napi::Number>();
    UWORD y = (uint32_t)info[2].As<Napi::Number>();

    GUI_ReadPng_WithOffset(path, x, y);

    PresentIfLive();

    return Napi::Boolean::New(env, true);
}

Napi::Boolean setPixel(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    UWORD x = (uint32_t)info[0].As<Napi::Number>();
    UWORD y = (uint32_t)info[1].As<Napi::Number>();

    UWORD color = (uint32_t)info[2].As<Napi::Number>();

    Paint_SetPixel(x, y, color);

    PresentIfLive();

	return Napi::Boolean::New(env, true);
}

Napi::Boolean drawImageBuffer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Array buffer = info[0].As<Napi::Array>();

    for (uint32_t y = 0; y < buffer.Length(); y++) {
        Napi::Array rowBuffer = buffer.Get(y).As<Napi::Array>();

        for (uint32_t x = 0; x < rowBuffer.Length(); x++) {
            UWORD color = (uint32_t)rowBuffer.Get(x).As<Napi::Number>();

            Paint_SetPixel(x, y, color);
        }
    }

    PresentIfLive();

	return Napi::Boolean::New(env, true);
}

Napi::Boolean clear(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    UWORD clearColor = 0x0000;

    if (info.Length() > 0) {
        clearColor = (uint32_t)info[0].As<Napi::Number>() << 8;
    }

    Paint_Clear(clearColor);

    PresentIfLive();

	return Napi::Boolean::New(env, true);
}

Napi::Boolean clearRectangle(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    UWORD x1 = (uint32_t)info[0].As<Napi::Number>();
    UWORD y1 = (uint32_t)info[1].As<Napi::Number>();
    UWORD x2 = (uint32_t)info[2].As<Napi::Number>();
    UWORD y2 = (uint32_t)info[3].As<Napi::Number>();

    UWORD color = 0x0000;
    if (info.Length() > 4) {
        color = (uint32_t)info[4].As<Napi::Number>() << 8;
    }

    if (x1 > x2) {
        UWORD tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        UWORD tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    if (x1 >= LCD_WIDTH) {
        printf("clearRectangle: x1 (%d) >= LCD_WIDTH (%d), clamping to 0\n", x1, LCD_WIDTH);
        x1 = 0;
    }
    if (y1 >= LCD_HEIGHT) {
        printf("clearRectangle: y1 (%d) >= LCD_HEIGHT (%d), clamping to 0\n", y1, LCD_HEIGHT);
        y1 = 0;
    }
    if (x2 >= LCD_WIDTH) {
        printf("clearRectangle: x2 (%d) >= LCD_WIDTH (%d), clamping to %d\n", x2, LCD_WIDTH, LCD_WIDTH - 1);
        x2 = LCD_WIDTH - 1;
    }
    if (y2 >= LCD_HEIGHT) {
        printf("clearRectangle: y2 (%d) >= LCD_HEIGHT (%d), clamping to %d\n", y2, LCD_HEIGHT, LCD_HEIGHT - 1);
        y2 = LCD_HEIGHT - 1;
    }

    if (x2 < x1 || y2 < y1) {
        DEBUG("clearRectangle: invalid rectangle after clamping");
        return Napi::Boolean::New(env, false);
    }

    Paint_ClearWindow(x1, y1, x2 + 1, y2 + 1, color);

    PresentIfLive();

    return Napi::Boolean::New(env, true);
}
