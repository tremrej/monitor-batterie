// ==================================================================================
// @file floatPicker.cpp
//
// ==================================================================================

#include "Adafruit_ILI9341.h"    // ILI9341_BLACK
#include "ILI9341_util.h"        // getTouchXY()
#include "ecranPrincipal.h"

// External declaration
void printFloatAt(float value, int width, int x, int y);

// Config button
#define configButtonX 50
#define configButtonY 210
#define configButtonW 80
#define configButtonH 31

// Reset button
#define resetButtonX 200
#define resetButtonY 200
#define resetButtonW 40
#define resetButtonH 40

// DC-DC charger enabled button
#define enableDcDcButtonX 278
#define enableDcDcButtonY 170-20
#define enableDcDcButtonW 40
#define enableDcDcButtonH 31

// Back button
#define backButtonX 320-62
#define backButtonY 210
#define backButtonW 60
#define backButtonH 31

// Dim button
#define dimButtonX 2
#define dimButtonY 210
#define dimButtonW 40
#define dimButtonH 31

Adafruit_GFX_Button EcranPrincipal::configButton_s     = Adafruit_GFX_Button();
Adafruit_GFX_Button EcranPrincipal::resetButton_s      = Adafruit_GFX_Button();
Adafruit_GFX_Button EcranPrincipal::enableDcDcButton_s = Adafruit_GFX_Button();
Adafruit_GFX_Button EcranPrincipal::backButton_s       = Adafruit_GFX_Button();
Adafruit_GFX_Button EcranPrincipal::dimButton_s        = Adafruit_GFX_Button();

EcranPrincipal::EcranPrincipal( Adafruit_GFX &tft
                              , AmpMeter &ampMeterStarter
                              , AmpMeter &ampMeterHouse
                              , int pinDim
                              , int pinIgnition
                              , int pinRelayDcDcEnable
                              , int pinRelayDcDcSlow)
    : tft_m(&tft)
    , ampMeterStarter_m(&ampMeterStarter)
    , ampMeterHouse_m  (&ampMeterHouse)
    , pinDim_m         (pinDim)
    , pinIgnition_m    (pinIgnition)
    , pinRelayDcDcEnable_m (pinRelayDcDcEnable)
    , pinRelayDcDcSlow_m   (pinRelayDcDcSlow)
{
}

EcranPrincipal::~EcranPrincipal()
{
}


void EcranPrincipal::init()
{
    configButton_s.initButtonUL( tft_m, configButtonX, configButtonY, configButtonW, configButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_YELLOW  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"config  ", 1, 2);

    resetButton_s.initButton( tft_m, resetButtonX, resetButtonY, resetButtonW, resetButtonH
                        , ILI9341_DARKGREY  // outline
                        , rgbTo565(128, 128, 128)
                        , rgbTo565(255, 255, 0)
                        //, ILI9341_GREEN  // fill
                        //, ILI9341_BLUE   // text
                        , (char *)"rst", 1, 2);
    enableDcDcButton_s.initButtonUL( tft_m, enableDcDcButtonX, enableDcDcButtonY
                                   , enableDcDcButtonW, enableDcDcButtonH
                        , ILI9341_DARKGREY  // outline
                        , 0xC958  // fill
                        , 0x79EE   // text
                        , (char *)"off", 1, 2);
    backButton_s.initButtonUL( tft_m, backButtonX, backButtonY, backButtonW, backButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_ORANGE  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"back  ", 1, 2);
    dimButton_s.initButtonUL( tft_m, dimButtonX, dimButtonY, dimButtonW, dimButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_DARKGREY  // fill
                        , ILI9341_BLACK   // text
                        , (char *)"dim", 1, 2);
    configButton_s.press(false);
}

void EcranPrincipal::checkUI(bool *goToConfigWindow)
{
    static bool toggle_s = false;
    static bool toggle2_s = false;
    int16_t x = 0;
    int16_t y = 0;
    *goToConfigWindow = false;
    if (getTouchXY(&x, &y))
    {
        if (resetButton_s.contains(x,y))
        {
            toggle_s = toggle_s? false: true;
            ampMeterStarter_m->resetAmpHour();
            ampMeterHouse_m->resetAmpHour();
            resetButton_s.drawButton(toggle_s);
            delay(100);
        }
        if (configButton_s.contains(x,y))
        {
            toggle2_s = toggle2_s? false: true;
            configButton_s.press(toggle2_s);
            configButton_s.drawButton(toggle2_s);
            //nextWindow_g = windowConfig_c;
            *goToConfigWindow = true;
            delay(100);
        }
        if (dimButton_s.contains(x,y))
        {
            adjustBacklight();
            delay(100);
        }
    }
}


void EcranPrincipal::drawStatic()
{
        tft_m->fillScreen(ILI9341_BLACK);
        tft_m->drawRect(0,0,tft_m->width(),tft_m->height(), rgbTo565(155,155,155));
        tft_m->drawFastHLine(0,18,tft_m->width(), rgbTo565(155,155,155));
        tft_m->setTextSize(1);
        tft_m->setCursor(5, 15);
        tft_m->setTextColor(ILI9341_WHITE);  tft_m->setTextSize(1);
        tft_m->println("Moniteur de batterie, v0.0");

        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("Volt:           V");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("Curr:           A");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("Pwr :           W");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("AH  :           AH");

        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("deltaT:         us");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("Time:            d h:m:s");

        resetButton_s.drawButton(false);
        enableDcDcButton_s.drawButton(enableDcDcButton_s.isPressed());
        configButton_s.drawButton(false);
        dimButton_s.drawButton(false);
        tft_m->setTextSize(1);
}

void EcranPrincipal::drawData( )
{
    printFloatAt(ampMeterStarter_m->getAvgBusVolt(), 1, 70, 33);
    printFloatAt(ampMeterStarter_m->getAvgCurrent(), 1, 70, 51);
    printFloatAt(ampMeterStarter_m->getAvgPower(), 1, 70, 69);
    printFloatAt(ampMeterStarter_m->getAmpHour(), 1, 70, 87);
    printTimeFromMilliSec(millis() - ampMeterStarter_m->getTimeSinceReset(), 70, 123);
}

void EcranPrincipal::adjustBacklight()
{
    char newLabel[10];
    uint16_t temp;
    if (dimLevel_m > 64)
    {
        dimLevel_m -= 64;
    }
    else if (dimLevel_m > 20)
    {
        dimLevel_m = 0;
    }
    else
    {
        // Rollover to full power
        dimLevel_m = 250;
    }
    analogWrite(pinDim_m, dimLevel_m);
    temp = dimLevel_m*100/255;
    sprintf(newLabel, "%d", temp);
    // Add '%' to the new lable
    temp = strlen(newLabel);
    newLabel[temp] = '%';
    newLabel[temp+1] = '\0';
    dimButton_s.changeLabel(newLabel);
    dimButton_s.drawButton(false);
}

