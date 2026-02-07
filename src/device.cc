#include "DEV_Config.h"
#include "GUI_Paint.h"

#include "addon.h"

#include "device.h"

UWORD *displayBuffer;
UDOUBLE Imagesize = LCD_HEIGHT*LCD_WIDTH*2;

Napi::Boolean initialize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    /* Module Init */
    if (DEV_ModuleInit() != 0){
        DEV_ModuleExit();

        printf("Failed to initialize display!\r\n");

        return Napi::Boolean::New(env, false);
    }

    LCD_SCAN_DIR LCD_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R

    LCD_1in44_Init(LCD_ScanDir);
    LCD_1in44_Clear(WHITE);

    if((displayBuffer = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Failed to create display buffer!\r\n");

        return Napi::Boolean::New(env, false);
    }

    Paint_NewImage(displayBuffer, LCD_WIDTH, LCD_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    printf("Initialized display!\r\n");

	return Napi::Boolean::New(env, true);
}

Napi::Boolean release(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    free(displayBuffer);
    displayBuffer = NULL;

    LCD_BL_0;

    printf("Released ressources\r\n");

	return Napi::Boolean::New(env, true);
}

Napi::Boolean changeDisplayState(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    bool on = false;

    if (info.Length() > 0) {
        on = info[0].As<Napi::Boolean>();

        Paint_Clear(BLACK);

        LCD_1in44_Display(displayBuffer);

        if (on) {
            LCD_BL_1;
        }
        else {
            LCD_BL_0;
        }
    }

	return Napi::Boolean::New(env, true);
}