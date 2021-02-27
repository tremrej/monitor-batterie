// ==================================================================================
// @file radioButton.h
//
// Declare a class to manage a window to pick selection within many (aka radio button)
// ==================================================================================
#ifndef radioButton_h
#define radioButton_h

#include "Adafruit_GFX.h"    // Adafruit_GFX_Button

// ==================================================================================
//  @class RadioButton
// 
// A screen to select a choice between multiple
// 
// ==================================================================================
class RadioButton
{
public:

    RadioButton( Adafruit_GFX &tft
               , char *title
               , char *listOfDesc[]
               , int  nbValue);
    ~RadioButton();

    // Return true if success, false failure
    bool init(int startIdx);

    // Check the UI and return the current value.
    // @retval true when "saved" pressed
    // Use getValue() to get the "saved" value.
    bool checkUI();

    int getValue();

    bool drawStatic();
    bool drawData();

private:

    Adafruit_GFX *tft_m;

    char  *title_m;
    char **listOfChoice_m;
    int    nbValue_m;

    Adafruit_GFX_Button **listOfButton_ma;
    Adafruit_GFX_Button saveButton_m;

    int currentIdx_m;
};

#endif  // radioButton_h
