#include <napi.h>

#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <signal.h>     //signal()

#include "device.h"
#include "display.h"
#include "keys.h"

using namespace Napi;

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Napi::Object device = Napi::Object::New(env);
    Napi::Object display = Napi::Object::New(env);
    Napi::Object keys = Napi::Object::New(env);

    device.Set(Napi::String::New(env, "initialize"), Napi::Function::New(env, initialize));
    device.Set(Napi::String::New(env, "release"), Napi::Function::New(env, release));
    device.Set(Napi::String::New(env, "changeDisplayState"), Napi::Function::New(env, changeDisplayState));

    display.Set(Napi::String::New(env, "writeText"), Napi::Function::New(env, writeText));
    display.Set(Napi::String::New(env, "drawBitmap"), Napi::Function::New(env, drawBitmap));
    display.Set(Napi::String::New(env, "setPixel"), Napi::Function::New(env, setPixel));
    display.Set(Napi::String::New(env, "drawImageBuffer"), Napi::Function::New(env, drawImageBuffer));
    display.Set(Napi::String::New(env, "clear"), Napi::Function::New(env, clear));

    keys.Set(Napi::String::New(env, "getState"), Napi::Function::New(env, getState));

	exports.Set(Napi::String::New(env, "Device"), device);
	exports.Set(Napi::String::New(env, "Display"), display);
	exports.Set(Napi::String::New(env, "Keys"), keys);

	return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
