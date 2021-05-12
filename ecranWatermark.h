// ==================================================================================
// @file ecranWatermark.h
//
// Declare a class to manage the watermark window.
// ==================================================================================
#ifndef ecranWatermark_h
#define ecranWatermark_h

#include "Adafruit_GFX.h"    // Adafruit_GFX_Button
#include "ampMeter.h"        // AmpMeter
//#include "chargerControl.h"
#include "monitorBatt.h"

// ==================================================================================
//  @class EcranWatermark
// 
// ==================================================================================
class EcranWatermark
{
public:

    EcranWatermark( Adafruit_GFX &tft
                  , AmpMeter &ampMeterStarter
                  , AmpMeter &ampMeterHouse
                  , AmpMeter &ampMeterAlternator
                  , AmpMeter &ampMeterSolar);
    ~EcranWatermark();

    void init();

    ActiveWindow_e checkUI();

    void drawStatic();
    void drawData();

private:
    Adafruit_GFX *tft_m;

    AmpMeter *ampMeterStarter_m;
    AmpMeter *ampMeterHouse_m;
    AmpMeter *ampMeterAlternator_m;
    AmpMeter *ampMeterSolar_m;

    Adafruit_GFX_Button resetButton_m;
    Adafruit_GFX_Button backButton_m;
    Adafruit_GFX_Button nextButton_m;
    Adafruit_GFX_Button backToMainButton_m;

};

#endif  // ecranWatermark_h
