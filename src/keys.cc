#include "DEV_Config.h"

#include <napi.h>

Napi::Array getState(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Array keys = Napi::Array::New(env);

    keys.Set(uint32_t(0), Napi::Boolean::New(env, GET_KEY_UP == 0));
    keys.Set(uint32_t(1), Napi::Boolean::New(env, GET_KEY_DOWN == 0));
    keys.Set(uint32_t(2), Napi::Boolean::New(env, GET_KEY_LEFT == 0));
    keys.Set(uint32_t(3), Napi::Boolean::New(env, GET_KEY_RIGHT == 0));
    keys.Set(uint32_t(4), Napi::Boolean::New(env, GET_KEY_PRESS == 0));
    keys.Set(uint32_t(5), Napi::Boolean::New(env, GET_KEY1 == 0));
    keys.Set(uint32_t(6), Napi::Boolean::New(env, GET_KEY2 == 0));
    keys.Set(uint32_t(7), Napi::Boolean::New(env, GET_KEY3 == 0));

    return keys;
}