// ==================================================================================
// @file ecranMotor.cpp
//
// ==================================================================================

#include "Adafruit_ILI9341.h"    // ILI9341_BLACK
#include "ILI9341_util.h"        // getTouchXY()
#include "ecranMotor.h"
#include "monitorBatt.h"
#include "persistent.h"          // motorSecond_m

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

EcranMotor::EcranMotor( Adafruit_GFX &tft
                      , Persistent   &persistent
                      , ChargerControl &chargerControl)
    : tft_m(&tft)
    , persistent_m (&persistent)
    , chargerControl_m(&chargerControl)
{
}

EcranMotor::~EcranMotor()
{
}


void EcranMotor::init()
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

ActiveWindow_e EcranMotor::checkUI()
{
    static bool toggle_s = false;
    int16_t x = 0;
    int16_t y = 0;
    ActiveWindow_e returnedWindow = windowMotor_c;

    if (getTouchXY(&x, &y))
    {
        if (resetButton_m.contains(x,y))
        {
            // No action
            toggle_s = toggle_s? false: true;
            resetButton_m.drawButton(toggle_s);
            drawStatic();
            drawData();
            returnedWindow = windowMotor_c;
            delay(100);
        }
        if (backButton_m.contains(x,y))
        {
            returnedWindow = windowWatermark_c;
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


void EcranMotor::drawStatic()
{
        tft_m->fillScreen(ILI9341_BLACK);
        tft_m->drawRect(0,0,tft_m->width(),tft_m->height(), rgbTo565(155,155,155));
        //tft_m->drawFastHLine(0,18,tft_m->width(), rgbTo565(155,155,155));

        tft_m->drawFastHLine(0,16+25,tft_m->width(), rgbTo565(155,155,155));

        // 25 is the increment of new line for font used (FreeMono9pt7b)
        tft_m->drawFastHLine(0,14+27+25,tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,14+27+(25*2),tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,14+27+(25*3),tft_m->width(), rgbTo565(90,90,90));

        tft_m->setTextSize(1,2);
        tft_m->setCursor(15*6, 25);     // 15 mm x 6 pixel per mm
        tft_m->setTextColor(ILI9341_WHITE);
        tft_m->println("Moteur");


        tft_m->setTextColor(ILI9341_WHITE);
        tft_m->setTextSize(1, 1);

        tft_m->setCursor(5, 33+25);
        tft_m->println("  Actuel(h:m:s):");

        tft_m->setCursor(5, 33+25+25);
        tft_m->println("   Total(h:m:s):");


//        resetButton_m.drawButton(false);
        backButton_m.drawButton(false);
        backToMainButton_m.drawButton(false);
        //nextButton_m.drawButton(false);
        tft_m->setTextSize(1);
}

void EcranMotor::drawData( )
{
    unsigned long ttt = 15 +    // second
                        5*60 +  // minutes
                        1122*60*60;   // heures
    tft_m->setTextSize(1, 1);

    printMotorTimeFromSec(chargerControl_m->currentMotorRunTime(),      200, 33+25);
    printMotorTimeFromSec(trunc(millis()/1000)+(1234*60*60),                  200, 33+25+25);
    //printMotorTimeFromMilliSec(ttt,                  200, 33+25+25);
    //printMotorTimeFromMilliSec(persistent_m->motorSecond_m,                  200, 33+25+25);
}

