// ==================================================================================
// @file floatPicker.cpp
//
// ==================================================================================

#include "Adafruit_ILI9341.h"    // ILI9341_BLACK
#include "ILI9341_util.h"        // getTouchXY()
#include "ecranPrincipal.h"
#include "monitorBatt.h"

// External declaration
void printFloatAt(float value, int width, int x, int y);

// Config button
#define configButtonX 50
#define configButtonY 210
#define configButtonW 80
#define configButtonH 31

// Reset button
#define resetButtonX 200
#define resetButtonY 215
#define resetButtonW 40
#define resetButtonH 40

// Next button
#define nextButtonX 320-62
#define nextButtonY 206
#define nextButtonW 60
#define nextButtonH 31
//#define nextButtonX 278
//#define nextButtonY 215
//#define nextButtonW 40
//#define nextButtonH 31

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
Adafruit_GFX_Button EcranPrincipal::nextButton_s = Adafruit_GFX_Button();
Adafruit_GFX_Button EcranPrincipal::backButton_s       = Adafruit_GFX_Button();
Adafruit_GFX_Button EcranPrincipal::dimButton_s        = Adafruit_GFX_Button();

EcranPrincipal::EcranPrincipal( Adafruit_GFX &tft
                              , AmpMeter &ampMeterStarter
                              , AmpMeter &ampMeterHouse
                              , AmpMeter &ampMeterAlternator
                              , AmpMeter &ampMeterSolar
                              , ChargerControl &chargerControl
                              , int pinDim
                              , int pinIgnition
                              , int pinRelayDcDcEnable
                              , int pinRelayDcDcSlow)
    : tft_m(&tft)
    , dimLevel_m(95)
    , ampMeterStarter_m(&ampMeterStarter)
    , ampMeterHouse_m  (&ampMeterHouse)
    , ampMeterAlternator_m  (&ampMeterAlternator)
    , ampMeterSolar_m  (&ampMeterSolar)
    , chargerControl_m (&chargerControl)
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
    nextButton_s.initButtonUL( tft_m, nextButtonX, nextButtonY
                                   , nextButtonW, nextButtonH
                        , ILI9341_DARKGREY  // outline
                        , 0xC958  // fill
                        , 0x79EE   // text
                        , (char *)"next", 1, 2);
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

ActiveWindow_e EcranPrincipal::checkUI()
{
    static bool toggle_s = false;
    static bool toggle2_s = false;
    int16_t x = 0;
    int16_t y = 0;
    ActiveWindow_e returnedWindow = windowEcran1_c;

    if (getTouchXY(&x, &y))
    {
        if (resetButton_s.contains(x,y))
        {
            toggle_s = toggle_s? false: true;
            ampMeterStarter_m->resetAmpHour();
            ampMeterHouse_m->resetAmpHour();
            resetButton_s.drawButton(toggle_s);
            drawStatic();
            drawData();
            returnedWindow = windowEcran1_c;
            delay(100);
        }
        if (configButton_s.contains(x,y))
        {
            toggle2_s = toggle2_s? false: true;
            configButton_s.press(toggle2_s);
            configButton_s.drawButton(toggle2_s);
            returnedWindow = windowConfig_c;
            delay(100);
        }
        if (nextButton_s.contains(x,y))
        {
            nextButton_s.drawButton();
            returnedWindow = windowWatermark_c;
            delay(100);
        }
        if (dimButton_s.contains(x,y))
        {
            adjustBacklight();
            delay(100);
        }
    }
    return returnedWindow;
}


void EcranPrincipal::drawStatic()
{
        tft_m->fillScreen(ILI9341_BLACK);
        tft_m->drawRect(0,0,tft_m->width(),tft_m->height(), rgbTo565(155,155,155));
        tft_m->drawFastHLine(0,18,tft_m->width(), rgbTo565(155,155,155));

        // Line below "Starter House"
        tft_m->drawFastHLine(0,18+25,tft_m->width(), rgbTo565(155,155,155));

        // 18 is the increment of new line for font used (FreeMono9pt7b)
        tft_m->drawFastHLine(0,18+27+18,tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,18+27+(18*2),tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,18+27+(18*3),tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,18+27+(18*4),tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,18+25+(18*5)+1,tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,18+25+(18*6)+4,tft_m->width(), rgbTo565(90,90,90));

        tft_m->setTextSize(1);
        tft_m->setCursor(5, 15);
        tft_m->setTextColor(ILI9341_WHITE);  tft_m->setTextSize(1);

#ifdef ARDUINO_AVR_MEGA2560
        tft_m->println("Moniteur de batterie, v0.3");
#else
        tft_m->println("Moniteur de batterie, t0.3");
#endif


        tft_m->setTextColor(ILI9341_WHITE);
        tft_m->setTextSize(1, 1);
        tft_m->setCursor(5, 33+25);

        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("            volt");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("            amp");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("            watt");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("            AH");

        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("Chgr start           amp");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("Chgr House           amp");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println(" Time            d h:m:s");

        resetButton_s.drawButton(false);
        nextButton_s.drawButton(nextButton_s.isPressed());
        configButton_s.drawButton(false);
        dimButton_s.drawButton(false);
        tft_m->setTextSize(1);
}

void EcranPrincipal::drawData( )
{
    // The color of the column header is green when charging and red when discharging.
    tft_m->setTextSize(1, 2);
    tft_m->setCursor(25, 15+18+5);
    if (ampMeterStarter_m->getAvgCurrent() < 0.0F)
    {
        tft_m->setTextColor(rgbTo565(60,225,70));    //green
        //tft_m->print(ttt);
    }
    else
    {
        tft_m->setTextColor(rgbTo565(240,80,55));    // red
        //tft_m->print(ttt);
    }
    tft_m->print("Starter");

    // Show charger state
    tft_m->fillRect(110, 21, 115, 20, rgbTo565(0,0,0));
    if (chargerControl_m->batterySelectorOnBoth())
    {
        tft_m->setTextColor(rgbTo565(235,64,55));    // red
        tft_m->print(" ==========");
    }
    else if (chargerControl_m->isChanging() &&
             chargerControl_m->isChangingSlow())
    {
        tft_m->setTextColor(rgbTo565(104,168,121));    // green-blue
        tft_m->print(" --> > >--");
    }
    else if (chargerControl_m->isChanging())
    {
        tft_m->setTextColor(rgbTo565(44,194,33));    // green
        tft_m->print(" -->>>>>>--");
    }

    tft_m->setCursor(230, tft_m->getCursorY());
    if (ampMeterHouse_m->getAvgCurrent() < 0.0F)
    {
        tft_m->setTextColor(rgbTo565(60,225,70));    // green
    }
    else
    {
        tft_m->setTextColor(rgbTo565(240,80,55));    // red
    }
    tft_m->print("House");

    printFloatAt(ampMeterStarter_m->getAvgBusVolt(), 1, 10, 33+25);
    printFloatAt(ampMeterStarter_m->getAvgCurrent(), 1, 10, 51+25);
    printFloatAt(ampMeterStarter_m->getAvgPower(), 1, 10, 69+25);
    printFloatAt(ampMeterStarter_m->getAmpHour(), 1, 10, 87+25);

    printFloatAt(ampMeterHouse_m->getAvgBusVolt(), 1, 210, 33+25);
    printFloatAt(ampMeterHouse_m->getAvgCurrent(), 1, 210, 51+25);
    printFloatAt(ampMeterHouse_m->getAvgPower(), 1, 210, 69+25);
    printFloatAt(ampMeterHouse_m->getAmpHour(), 1, 210, 87+25);

    printFloatAt(ampMeterAlternator_m->getAvgCurrent(), 1, 118, 87+25+18);
    printFloatAt(ampMeterSolar_m->getAvgCurrent(),      1, 118, 87+25+18+18);

    printTimeFromMilliSec(millis() - ampMeterStarter_m->getTimeSinceReset(), 70, 123+25+(18*1));
}

void EcranPrincipal::adjustBacklight()
{
    char newLabel[10];
    uint16_t temp;
    if (dimLevel_m > 80)
    {
        dimLevel_m -= 80;
    }
    else if (dimLevel_m > 9)
    {
        dimLevel_m = 0;
    }
    else
    {
        // Rollover to full power
        dimLevel_m = 254;
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

