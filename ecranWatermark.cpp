// ==================================================================================
// @file floatPicker.cpp
//
// ==================================================================================

#include "Adafruit_ILI9341.h"    // ILI9341_BLACK
#include "ILI9341_util.h"        // getTouchXY()
#include "ecranWatermark.h"
#include "monitorBatt.h"

// External declaration
void printFloatAt(float value, int width, int x, int y);

// Back button
#define backButtonX 2
#define backButtonY 206
#define backButtonW 60
#define backButtonH 31

// Next button
#define nextButtonX 320-62
#define nextButtonY 206
#define nextButtonW 60
#define nextButtonH 31

// Back to main button
#define backToMainButtonX 320/2 - 60/3
#define backToMainButtonY 206
#define backToMainButtonW 60
#define backToMainButtonH 31

// Reset button
#define resetButtonX 260
#define resetButtonY 180
#define resetButtonW 85
#define resetButtonH 40

EcranWatermark::EcranWatermark( Adafruit_GFX &tft
                              , AmpMeter &ampMeterStarter
                              , AmpMeter &ampMeterHouse
                              , AmpMeter &ampMeterAlternator
                              , AmpMeter &ampMeterSolar)
    : tft_m(&tft)
    , ampMeterStarter_m(&ampMeterStarter)
    , ampMeterHouse_m  (&ampMeterHouse)
    , ampMeterAlternator_m  (&ampMeterAlternator)
    , ampMeterSolar_m  (&ampMeterSolar)
{
}

EcranWatermark::~EcranWatermark()
{
}


void EcranWatermark::init()
{

    backButton_m.initButtonUL( tft_m, backButtonX, backButtonY, backButtonW, backButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_ORANGE  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"back  ", 1, 2);
    nextButton_m.initButtonUL( tft_m, nextButtonX, nextButtonY, nextButtonW, nextButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_ORANGE  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"next  ", 1, 2);
    backToMainButton_m.initButtonUL( tft_m, backToMainButtonX, backToMainButtonY, backToMainButtonW, backToMainButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_ORANGE  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"main  ", 1, 2);


    resetButton_m.initButton( tft_m, resetButtonX, resetButtonY, resetButtonW, resetButtonH
                        , ILI9341_DARKGREY  // outline
                        , rgbTo565(128, 128, 128)
                        , rgbTo565(255, 255, 0)
                        //, ILI9341_GREEN  // fill
                        //, ILI9341_BLUE   // text
                        , (char *)"reset", 1, 2);
}

ActiveWindow_e EcranWatermark::checkUI()
{
    static bool toggle_s = false;
    int16_t x = 0;
    int16_t y = 0;
    ActiveWindow_e returnedWindow = windowWatermark_c;

    if (getTouchXY(&x, &y))
    {
        if (resetButton_m.contains(x,y))
        {
            toggle_s = toggle_s? false: true;
            ampMeterStarter_m->resetWatermark();
            ampMeterHouse_m->resetWatermark();
            ampMeterAlternator_m->resetWatermark();
            ampMeterSolar_m->resetWatermark();
            resetButton_m.drawButton(toggle_s);
            drawStatic();
            drawData();
            returnedWindow = windowWatermark_c;
            delay(100);
        }
        if (backButton_m.contains(x,y))
        {
            returnedWindow = windowEcran1_c;
            delay(100);
        }
        if (backToMainButton_m.contains(x,y))
        {
            returnedWindow = windowEcran1_c;
            delay(100);
        }
        if (nextButton_m.contains(x,y))
        {
            returnedWindow = windowMotor_c;
            delay(100);
        }
    }
    return returnedWindow;
}


void EcranWatermark::drawStatic()
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

        tft_m->println("          Watermark");

        tft_m->setTextColor(ILI9341_WHITE);
        tft_m->setTextSize(1, 1);
        tft_m->setCursor(5, 33+25);

        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("          voltHigh");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("          voltLow");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("          ampHigh");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("          ampLow");

        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("Chgr start           ampHigh");
        tft_m->setCursor(5, tft_m->getCursorY());
        tft_m->println("Chgr House           ampHigh");
        tft_m->setCursor(5, tft_m->getCursorY());

        resetButton_m.drawButton(false);
        backButton_m.drawButton(false);
        backToMainButton_m.drawButton(false);
        nextButton_m.drawButton(false);
        tft_m->setTextSize(1);
}

void EcranWatermark::drawData( )
{
    // The color of the column header is green when charging and red when discharging.
    tft_m->setTextSize(1, 2);
    tft_m->setCursor(25, 15+18+5);
    if (ampMeterStarter_m->getAvgCurrent() < 0.0F)
    {
        tft_m->setTextColor(rgbTo565(60,225,70));    //green
    }
    else
    {
        tft_m->setTextColor(rgbTo565(240,80,55));    // red
    }
    tft_m->print("Starter");

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

    float voltHigh, voltLow, ampHigh, ampLow = 0.0;

    ampMeterStarter_m->getBusVoltWatermark(&voltHigh, &voltLow);
    ampMeterStarter_m->getCurrentWatermark(&ampHigh, &ampLow);

    printFloatAt(voltHigh, 1, 10, 33+25);
    printFloatAt(voltLow, 1, 10, 51+25);
    printFloatAt(ampHigh, 1, 10, 69+25);
    printFloatAt(ampLow, 1, 10, 87+25);

    ampMeterHouse_m->getBusVoltWatermark(&voltHigh, &voltLow);
    ampMeterHouse_m->getCurrentWatermark(&ampHigh, &ampLow);

    printFloatAt(voltHigh, 1, 210, 33+25);
    printFloatAt(voltLow, 1, 210, 51+25);
    printFloatAt(ampHigh, 1, 210, 69+25);
    printFloatAt(ampLow, 1, 210, 87+25);

    ampMeterAlternator_m->getCurrentWatermark(&ampHigh, &ampLow);
    printFloatAt(ampHigh, 1, 118, 87+25+18);

    ampMeterSolar_m->getCurrentWatermark(&ampHigh, &ampLow);
    printFloatAt(ampHigh,      1, 118, 87+25+18+18);
}

