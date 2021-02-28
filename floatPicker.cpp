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
#define saveButtonX 200
#define saveButtonY 190
#define saveButtonW 120
#define saveButtonH 37

// Cancel button
#define cancelButtonX 30
#define cancelButtonY 190
#define cancelButtonW 100
#define cancelButtonH 37

FloatPicker::FloatPicker( Adafruit_GFX &tft
                        , char * title, float min, float max, float increment)
    : tft_m(&tft)
    , title_m (title)
    , min_m (min)
    , max_m (max)
    , increment_m (increment)
    , currentValue_m (min_m)
    , upButton_m ()
    , downButton_m ()
    , saveButton_m ()
    , cancelButton_m ()
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
    upButton_m.initButtonUL( tft_m, upButtonX, upButtonY
                                  , upButtonW, upButtonH
                            , ILI9341_WHITE  // outline
                            , ILI9341_BLUE  // fill
                            , ILI9341_GREEN   // text
                            , (char *)"up", 1, 2);
    downButton_m.initButtonUL( tft_m, downButtonX, downButtonY
                                    , downButtonW, downButtonH
                            , ILI9341_WHITE  // outline
                            , ILI9341_BLUE  // fill
                            , ILI9341_GREEN   // text
                            , (char *)"dn", 1, 2);
    saveButton_m.initButtonUL( tft_m, saveButtonX, saveButtonY
                                    , saveButtonW, saveButtonH
                            , ILI9341_WHITE  // outline
                            , ILI9341_BLUE  // fill
                            , ILI9341_GREEN   // text
                            , (char *)"ok", 1, 2);
    cancelButton_m.initButtonUL( tft_m, cancelButtonX, cancelButtonY
                                      , cancelButtonW, cancelButtonH
                            , ILI9341_WHITE  // outline
                            , ILI9341_BLUE  // fill
                            , ILI9341_GREEN   // text
                            , (char *)"cancel", 1, 2);
    return true;
}

Action_e FloatPicker::checkUI()
{
    int16_t x = 0;
    int16_t y = 0;
    Action_e rc = noOp_c;

    if (getTouchXY(&x, &y))
    {
        if (upButton_m.contains(x,y))
        {
            currentValue_m += increment_m;
            if (currentValue_m > max_m) currentValue_m = max_m;
            drawData();
            delay(25);
        }
        if (downButton_m.contains(x,y))
        {
            currentValue_m -= increment_m;
            if (currentValue_m < min_m) currentValue_m = min_m;
            drawData();
            delay(25);
        }
        if (saveButton_m.contains(x,y))
        {
            rc = save_c;
            delay(100);
        }
        if (cancelButton_m.contains(x,y))
        {
            rc = cancel_c;
            delay(100);
        }
    }
    return rc;
}

float FloatPicker::getValue()
{
    return currentValue_m;
}

float FloatPicker::setValue( float value)
{
    currentValue_m = value;
    return currentValue_m;
}

bool FloatPicker::drawStatic()
{
    tft_m->fillScreen(ILI9341_BLACK);
    tft_m->setTextSize(1);
    tft_m->setCursor(25, 25);
    tft_m->setTextColor(ILI9341_WHITE);
    tft_m->print(title_m);

    upButton_m.drawButton();
    downButton_m.drawButton();
    saveButton_m.drawButton();
    cancelButton_m.drawButton();

    return drawData();
}

bool FloatPicker::drawData()
{
    printFloatAt(currentValue_m, 2, 130, 113);
    return true;
}
