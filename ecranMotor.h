// ==================================================================================
// @file ecranMotor.h
//
// Declare a class to manage the Motor window.
// ==================================================================================
#ifndef ecranMotor_h
#define ecranMotor_h

#include "Adafruit_GFX.h"    // Adafruit_GFX_Button
#include "ampMeter.h"        // AmpMeter
#include "chargerControl.h"
#include "monitorBatt.h"

// ==================================================================================
//  @class EcranWatermark
// 
// ==================================================================================
class EcranMotor
{
public:

    EcranMotor( Adafruit_GFX &tft
              , Persistent     &persistent
              , ChargerControl &chargerControl);
    ~EcranMotor();

    void init();

    ActiveWindow_e checkUI();

    void drawStatic();
    void drawData();

private:
    Adafruit_GFX *tft_m;

    Persistent   *persistent_m;
    ChargerControl *chargerControl_m;

    Adafruit_GFX_Button resetButton_m;
    Adafruit_GFX_Button backButton_m;
    Adafruit_GFX_Button nextButton_m;
    Adafruit_GFX_Button backToMainButton_m;

};

#endif  // ecranMotor_h
