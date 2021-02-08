// ==================================================================================
// @file floatPicker.cpp
//
// ==================================================================================

#include "Adafruit_ILI9341.h"    // ILI9341_BLACK
#include "floatPicker.h"
#include "ILI9341_util.h"        // getTouchXY()

// External declaration
void printFloatAt(float value, int width, int x, int y);

// Up button
#define upButtonX 278
#define upButtonY 50
#define upButtonW 40
#define upButtonH 40

// Down button
#define downButtonX 278
#define downButtonY 125
#define downButtonW 40
#define downButtonH 40

// Save button
#define saveButtonX 30
#define saveButtonY 190
#define saveButtonW 40
#define saveButtonH 40

Adafruit_GFX_Button FloatPicker::upButton_s   = Adafruit_GFX_Button();
Adafruit_GFX_Button FloatPicker::downButton_s = Adafruit_GFX_Button();
Adafruit_GFX_Button FloatPicker::saveButton_s = Adafruit_GFX_Button();

FloatPicker::FloatPicker( Adafruit_GFX &tft
                        , char * title, float min, float max, float increment)
{
    tft_m = &tft;
    title_m = title;
    min_m = min;
    max_m = max;
    increment_m = increment;
    currentValue_m = min_m;

}

FloatPicker::~FloatPicker()
{
}

bool FloatPicker::init(float startValue)
{
    currentValue_m = startValue;
    upButton_s.initButtonUL( tft_m, upButtonX, upButtonY
                                 , upButtonW, upButtonH
                            , ILI9341_WHITE  // outline
                            , ILI9341_BLUE  // fill
                            , ILI9341_GREEN   // text
                            , (char *)"up", 1, 2);
    downButton_s.initButtonUL( tft_m, downButtonX, downButtonY
                                 , downButtonW, downButtonH
                            , ILI9341_WHITE  // outline
                            , ILI9341_BLUE  // fill
                            , ILI9341_GREEN   // text
                            , (char *)"dn", 1, 2);
    saveButton_s.initButtonUL( tft_m, saveButtonX, saveButtonY
                                 , saveButtonW, saveButtonH
                            , ILI9341_WHITE  // outline
                            , ILI9341_BLUE  // fill
                            , ILI9341_GREEN   // text
                            , (char *)"ok", 1, 2);
    return true;
}

bool FloatPicker::checkUI()
{
    int16_t x = 0;
    int16_t y = 0;
    bool rc = false;
    if (getTouchXY(&x, &y))
    {
        if (upButton_s.contains(x,y))
        {
            currentValue_m += increment_m;
            if (currentValue_m > max_m) currentValue_m = max_m;
            drawData();
            delay(25);
        }
        if (downButton_s.contains(x,y))
        {
            currentValue_m -= increment_m;
            if (currentValue_m < min_m) currentValue_m = min_m;
            drawData();
            delay(25);
        }
        if (saveButton_s.contains(x,y))
        {
            rc = true;
            delay(100);
        }
    }
    return rc;
}

float FloatPicker::getValue()
{
    return currentValue_m;
}

bool FloatPicker::drawStatic()
{
    tft_m->fillScreen(ILI9341_BLACK);
    tft_m->setTextSize(1);
    tft_m->setCursor(25, 25);
    tft_m->setTextColor(ILI9341_WHITE);
    tft_m->print(title_m);

    upButton_s.drawButton();
    downButton_s.drawButton();
    saveButton_s.drawButton();

    drawData();

}

bool FloatPicker::drawData()
{
    printFloatAt(currentValue_m, 2, 130, 113);
}
