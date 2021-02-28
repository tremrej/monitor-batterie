// ==================================================================================
// @file ampMeter.h
//
// Declare a class to manage a window to pick a float number
// The current implementation uses a tft Adafruit_ILI9341 and a touch screen Adafruit_STMPE610
// ==================================================================================
#ifndef floatPicker_h
#define floatPicker_h

#include "Adafruit_GFX.h"    // Adafruit_GFX_Button

enum Action_e {
    save_c,
    cancel_c,
    noOp_c
};


// ==================================================================================
//  @class FloatPicker
// 
// A screen to enter a float number
// 
// ==================================================================================
class FloatPicker
{
public:

    FloatPicker( Adafruit_GFX &tft
               , char *title
               , float min
               , float max
               , float increment);
    ~FloatPicker();

    // Return true if success, false failure
    bool init(float startValue);

    // Check the UI and return the current value.
    // @retval true when "saved" pressed
    // Use getValue() to get the "saved" value.
    Action_e checkUI();

    // These two operator are not compiling...
    //operator float () { return currentValue_m; }
    //FloatPicker &operator=(float val) { currentValue_m = val; return *this; }

    float getValue();
    float setValue(float value);

    bool drawStatic();
    bool drawData();

private:

    Adafruit_GFX *tft_m;

    char *title_m;
    float min_m;
    float max_m;
    float increment_m;
    float currentValue_m;

    Adafruit_GFX_Button upButton_m;
    Adafruit_GFX_Button downButton_m;
    Adafruit_GFX_Button saveButton_m;
    Adafruit_GFX_Button cancelButton_m;
};

#endif  // floatPicker_h
