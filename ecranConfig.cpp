// ==================================================================================
// @file ecranConfig.cpp
//
// ==================================================================================

#include "Adafruit_ILI9341.h"    // ILI9341_BLACK
#include "ILI9341_util.h"        // getTouchXY()
#include "ecranConfig.h"

// External declaration
void printFloatAt(float value, int width, int x, int y);

// inVolt button
#define inVoltButtonX 320-115
#define inVoltButtonY 45
#define inVoltButtonW 80
#define inVoltButtonH 20

// delay button
#define delayButtonX 320-115
#define delayButtonY 45+25
#define delayButtonW 80
#define delayButtonH 20

// allDeadZone button
#define allDeadZoneButtonX 320-115
#define allDeadZoneButtonY 45+25+25
#define allDeadZoneButtonW 80
#define allDeadZoneButtonH 20

// Back button
#define backButtonX 320-62
#define backButtonY 206
#define backButtonW 60
#define backButtonH 31

EcranConfig::EcranConfig( Adafruit_GFX &tft
                        , Persistent   &persistent
                        , FloatPicker  &inVoltThreshold
                        , FloatPicker  &chargeStartDelay
                        , FloatPicker  &allDeadZone)
    : tft_m(&tft)
    , persistent_m (&persistent)
    , inVoltThreshold_m (&inVoltThreshold)
    , chargeStartDelay_m (&chargeStartDelay)
    , allDeadZone_m (&allDeadZone)
    , inVoltButton_m()
    , delayButton_m()
    , allDeadZoneButton_m()
    , backButton_m()
    , activeWindow_m (windowConfig_c)
    , nextWindow_m (windowConfig_c)
{
}

EcranConfig::~EcranConfig()
{
}


void EcranConfig::init()
{
    inVoltButton_m.initButtonUL( tft_m, inVoltButtonX, inVoltButtonY, inVoltButtonW, inVoltButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_YELLOW  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"x", 1, 2);

    delayButton_m.initButtonUL( tft_m, delayButtonX, delayButtonY, delayButtonW, delayButtonH
                              , ILI9341_DARKGREY  // outline
                              , ILI9341_YELLOW  // fill
                              , ILI9341_BLUE   // text
                              , (char *)"x", 1, 2);

    allDeadZoneButton_m.initButtonUL( tft_m, allDeadZoneButtonX, allDeadZoneButtonY, allDeadZoneButtonW
                                                                                   , allDeadZoneButtonH
                              , ILI9341_DARKGREY  // outline
                              , ILI9341_YELLOW  // fill
                              , ILI9341_BLUE   // text
                              , (char *)"x", 1, 2);


    backButton_m.initButtonUL( tft_m, backButtonX, backButtonY, backButtonW, backButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_ORANGE  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"back  ", 1, 2);
    //inVoltButton_m.press(false);
}

void EcranConfig::processChangeOfWindow(ActiveWindow_e window)
{
    switch (window)
    {
        case windowConfig_c:
        {
            drawStatic();
            drawData();
            break;
        }
        case windowPickDcDcInVoltThres_c:
        {
            inVoltThreshold_m->drawStatic();
            break;
        }
        case windowPickDcDcDelay_c:
        {
            chargeStartDelay_m->drawStatic();
            break;
        }
        case windowPickAllSelectDeadzone_c:
        {
            allDeadZone_m->drawStatic();
            break;
        }
        default: break;
    }
}

ActiveWindow_e EcranConfig::checkUI()
{
    int16_t x = 0;
    int16_t y = 0;
    ActiveWindow_e returnedWindow = windowConfig_c;

    if (nextWindow_m != activeWindow_m)
    {
        // We're changing window.
        processChangeOfWindow(nextWindow_m);
        activeWindow_m = nextWindow_m;
    }

    if (activeWindow_m == windowConfig_c)
    {
        if (getTouchXY(&x, &y))
        {
            if (inVoltButton_m.contains(x,y))
            {
                nextWindow_m = windowPickDcDcInVoltThres_c;
                delay(100);
            }
            if (delayButton_m.contains(x,y))
            {
                nextWindow_m = windowPickDcDcDelay_c;
                delay(100);
            }
            if (allDeadZoneButton_m.contains(x,y))
            {
                nextWindow_m = windowPickAllSelectDeadzone_c;
                delay(100);
            }
            if (backButton_m.contains(x,y))
            {
                returnedWindow = windowEcran1_c;
                delay(100);
            }
        }
    }
    else if (activeWindow_m == windowPickDcDcInVoltThres_c)
    {
        float ttt = inVoltThreshold_m->getValue();

        if (inVoltThreshold_m->checkUI())
        {
            // Value picked. Let's go back to the main window.
            nextWindow_m = windowConfig_c;
        }
        if (ttt != inVoltThreshold_m->getValue())
        {
            // Save the new value.
            persistent_m->setInputVoltThreshold(inVoltThreshold_m->getValue());
        }
    }
    else if (activeWindow_m == windowPickDcDcDelay_c)
    {
        float ttt = chargeStartDelay_m->getValue();

        if (chargeStartDelay_m->checkUI())
        {
            // Value picked. Let's go back to the main window.
            nextWindow_m = windowConfig_c;
        }
        if (ttt != chargeStartDelay_m->getValue())
        {
            // Save the new value.
            persistent_m->setDelay(chargeStartDelay_m->getValue());
        }
    }
    else if (activeWindow_m == windowPickAllSelectDeadzone_c)
    {
        float ttt = allDeadZone_m->getValue();

        if (allDeadZone_m->checkUI())
        {
            // Value picked. Let's go back to the main window.
            nextWindow_m = windowConfig_c;
        }
        if (ttt != allDeadZone_m->getValue())
        {
            // Save the new value.
            persistent_m->setAllDeadZone(allDeadZone_m->getValue());
        }
    }

    return returnedWindow;
}


void EcranConfig::drawStatic()
{
        tft_m->fillScreen(ILI9341_BLACK);
        tft_m->drawRect(0,0,tft_m->width(),tft_m->height(), rgbTo565(155,155,155));
        //tft_m->drawFastHLine(0,18,tft_m->width(), rgbTo565(155,155,155));

        tft_m->drawFastHLine(0,16+25,tft_m->width(), rgbTo565(155,155,155));

        // 25 is the increment of new line for font used (FreeMono9pt7b)
        tft_m->drawFastHLine(0,14+27+25,tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,14+27+(25*2),tft_m->width(), rgbTo565(90,90,90));
        tft_m->drawFastHLine(0,14+27+(25*3),tft_m->width(), rgbTo565(90,90,90));
        //tft_m->drawFastHLine(0,18+27+(25*4),tft_m->width(), rgbTo565(90,90,90));
        //tft_m->drawFastHLine(0,18+25+(25*5)+1,tft_m->width(), rgbTo565(90,90,90));
        //tft_m->drawFastHLine(0,18+25+(25*6)+4,tft_m->width(), rgbTo565(90,90,90));

        tft_m->setTextSize(1,2);
        tft_m->setCursor(15*6, 25);     // 15 mm x 6 pixel per mm
        tft_m->setTextColor(ILI9341_WHITE);
        tft_m->println("Configuration");


        tft_m->setTextColor(ILI9341_WHITE);
        tft_m->setTextSize(1, 1);

        tft_m->setCursor(5, 33+25);
        tft_m->println("Input volt thresh:");

        tft_m->setCursor(5, 33+25+25);
        tft_m->println("Start delay(sec):");

        tft_m->setCursor(5, 33+25+25+25);
        tft_m->println("All deadzone:");

        //inVoltButton_m.drawButton(false);
        backButton_m.drawButton(false);
        tft_m->setTextSize(1);
}

void EcranConfig::drawData( )
{
    tft_m->setTextSize(1, 1);
    //tft_m->setCursor(200, 33+25);

    printFloatAt(inVoltThreshold_m->getValue(), 1, 200, 33+25);

    printFloatAt(chargeStartDelay_m->getValue(), 1, 200, 33+25+25);

    printFloatAt(allDeadZone_m->getValue(), 1, 200, 33+25+25+25);
}

