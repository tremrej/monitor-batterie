// ==================================================================================
// @file radioButton.cpp
//
// ==================================================================================

#include "Adafruit_ILI9341.h"    // ILI9341_BLACK
#include "radioButton.h"
#include "ILI9341_util.h"        // getTouchXY()

// Save button
#define saveButtonX 30

#define saveButtonY 190
#define saveButtonW 40
#define saveButtonH 40

RadioButton::RadioButton( Adafruit_GFX &tft
                        , char *title
                        , char *listOfDesc[]
                        , int  nbValue)
    : tft_m (&tft)
    , title_m (title)
    , listOfChoice_m(listOfDesc)
    , nbValue_m (nbValue)
    , listOfButton_ma (0)
    , saveButton_m ()
    , currentIdx_m(0)
{
}

RadioButton::~RadioButton()
{
}

bool RadioButton::init(int startIdx)
{
    currentIdx_m = startIdx;
    saveButton_m.initButtonUL( tft_m, saveButtonX, saveButtonY
                                    , saveButtonW, saveButtonH
                            , ILI9341_WHITE  // outline
                            , ILI9341_BLUE  // fill
                            , ILI9341_GREEN   // text
                            , (char *)"ok", 1, 2);

    listOfButton_ma = (Adafruit_GFX_Button **) malloc (nbValue_m * sizeof(Adafruit_GFX_Button *));
    for (int idx = 0; idx < nbValue_m; idx++)
    {
        listOfButton_ma[idx] = new Adafruit_GFX_Button();
        Serial.print("button: ");
        //Serial.println(&listOfButton_ma[idx], HEX);


        listOfButton_ma[idx]->initButtonUL( tft_m, 240, 60+(25*idx), 30, 20
                            , ILI9341_WHITE  // outline
                            , rgbTo565(222, 115, 100) // fill
                            , rgbTo565(110, 90, 90) // text
                            , (char *)"", 1, 2);
    }
    return true;
}

bool RadioButton::checkUI()
{
    int16_t x = 0;
    int16_t y = 0;
    bool rc = false;
    if (getTouchXY(&x, &y))
    {
        for (int idx = 0; idx < nbValue_m; idx++)
        {
            if (listOfButton_ma[idx]->contains(x,y))
            {
                currentIdx_m = idx;
                drawStatic();
                delay(25);
                break;
            }
        }
        if (saveButton_m.contains(x,y))
        {
            rc = true;
            delay(100);
        }
    }
    return rc;
}

int RadioButton::getValue()
{
    return currentIdx_m;
}

bool RadioButton::drawStatic()
{
    tft_m->fillScreen(ILI9341_BLACK);
    tft_m->drawRect(0,0,tft_m->width(),tft_m->height(), rgbTo565(155,155,155));
    tft_m->drawFastHLine(0,18+25,tft_m->width(), rgbTo565(155,155,155));
    tft_m->setTextSize(1,2);
    tft_m->setCursor(85, 28);
    tft_m->setTextColor(ILI9341_WHITE);
    tft_m->print(title_m);
 
    saveButton_m.drawButton();

    for (int idx = 0; idx < nbValue_m; idx++)
    {
        tft_m->setCursor(50, 70+(25*idx));
        tft_m->setTextColor(ILI9341_WHITE);
        tft_m->setTextSize(1);
        tft_m->print(listOfChoice_m[idx]);
        if (idx == currentIdx_m)
        {
            listOfButton_ma[idx]->changeLabel(" x");
        }
        else
        {
            listOfButton_ma[idx]->changeLabel("");
        }
        listOfButton_ma[idx]->drawButton(idx != currentIdx_m);
    }

    return drawData();
}

bool RadioButton::drawData()
{
    // Nothing to do
    return true;
}
