#include <napi.h>

#ifndef DISPLAY_H
#define DISPLAY_H

Napi::Boolean writeText(const Napi::CallbackInfo& info);
Napi::Boolean drawBitmap(const Napi::CallbackInfo& info);
Napi::Boolean setPixel(const Napi::CallbackInfo& info);
Napi::Boolean drawImageBuffer(const Napi::CallbackInfo& info);
Napi::Boolean clear(const Napi::CallbackInfo& info);

#endif