#include <napi.h>

#include "LCD_1in44.h"
#include "fonts.h"
#include "GUI_BMP.h"
#include "GUI_Paint.h"

#include "addon.h"

#include "display.h"

Napi::Boolean writeText(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    char* text = (char*)info[0].As<Napi::String>().Utf8Value().c_str();

    UWORD x = (uint32_t)info[1].As<Napi::Number>();
    UWORD y = (uint32_t)info[2].As<Napi::Number>();

    UWORD color = (uint32_t)info[3].As<Napi::Number>();
    UWORD backgroundColor = (uint32_t)info[4].As<Napi::Number>();

    Paint_DrawString_EN(x, y, text, &Font12, color, backgroundColor);

    LCD_1in44_Display(displayBuffer);

	return Napi::Boolean::New(env, true);
}

Napi::Boolean drawBitmap(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    char* path = (char*)info[0].As<Napi::String>().Utf8Value().c_str();

    GUI_ReadBmp(path);

    LCD_1in44_Display(displayBuffer);

	return Napi::Boolean::New(env, true);
}

Napi::Boolean setPixel(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    UWORD x = (uint32_t)info[0].As<Napi::Number>();
    UWORD y = (uint32_t)info[1].As<Napi::Number>();

    UWORD color = (uint32_t)info[2].As<Napi::Number>();

    Paint_SetPixel(x, y, color);

    LCD_1in44_Display(displayBuffer);

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

    LCD_1in44_Display(displayBuffer);

	return Napi::Boolean::New(env, true);
}

Napi::Boolean clear(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    UWORD clearColor = 0x0000;

    if (info.Length() > 0) {
        clearColor = (uint32_t)info[0].As<Napi::Number>() << 8;
    }

    Paint_Clear(clearColor);

    LCD_1in44_Display(displayBuffer);

	return Napi::Boolean::New(env, true);
}