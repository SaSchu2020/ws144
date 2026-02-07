#include <napi.h>

#ifndef DEVICE_H
#define DEVICE_H

Napi::Boolean initialize(const Napi::CallbackInfo& info);
Napi::Boolean release(const Napi::CallbackInfo& info);
Napi::Boolean changeDisplayState(const Napi::CallbackInfo& info);

#endif