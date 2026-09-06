#include <napi.h>

#ifndef DISPLAY_H
#define DISPLAY_H

Napi::Boolean writeText(const Napi::CallbackInfo& info);
Napi::Boolean drawBitmap(const Napi::CallbackInfo& info);
Napi::Boolean drawPng(const Napi::CallbackInfo& info);
Napi::Boolean setPixel(const Napi::CallbackInfo& info);
Napi::Boolean drawImageBuffer(const Napi::CallbackInfo& info);
Napi::Boolean clear(const Napi::CallbackInfo& info);
Napi::Boolean clearRectangle(const Napi::CallbackInfo& info);
Napi::Boolean drawBitmapAt(const Napi::CallbackInfo& info);
Napi::Boolean drawPngAt(const Napi::CallbackInfo& info);
Napi::Boolean beginDraw(const Napi::CallbackInfo& info);
Napi::Boolean endDraw(const Napi::CallbackInfo& info);

#endif