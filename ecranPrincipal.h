// ==================================================================================
// @file ecranPrincipal.h
//
// Declare a class to manage the main window.
// ==================================================================================
#ifndef ecranPrincipal_h
#define ecranPrincipal_h

#include "Adafruit_GFX.h"    // Adafruit_GFX_Button
#include "ampMeter.h"        // AmpMeter
#include "chargerControl.h"
#include "monitorBatt.h"

// ==================================================================================
//  @class EcranPrincipal
// 
// ==================================================================================
class EcranPrincipal
{
public:

    EcranPrincipal( Adafruit_GFX &tft
                  , AmpMeter &ampMeterStarter
                  , AmpMeter &ampMeterHouse
                  , AmpMeter &ampMeterAlternator
                  , AmpMeter &ampMeterSolar
                  , ChargerControl &chargerControl
                  , int pinDim
                  , int pinIgnition
                  , int pinRelayDcDcEnable
                  , int pinRelayDcDcSlow);
    ~EcranPrincipal();

    void init();

    ActiveWindow_e checkUI();

    void drawStatic();
    void drawData();
    void adjustBacklight();

private:
    Adafruit_GFX *tft_m;

    unsigned int dimLevel_m = 64;   // Backlight dim level, 0..255

    static Adafruit_GFX_Button configButton_s;
    static Adafruit_GFX_Button resetButton_s;
    static Adafruit_GFX_Button enableDcDcButton_s;
    static Adafruit_GFX_Button backButton_s;
    static Adafruit_GFX_Button dimButton_s;

    AmpMeter *ampMeterStarter_m;
    AmpMeter *ampMeterHouse_m;
    AmpMeter *ampMeterAlternator_m;
    AmpMeter *ampMeterSolar_m;
    ChargerControl *chargerControl_m;
    int       pinDim_m;
    int       pinIgnition_m;
    int       pinRelayDcDcEnable_m;
    int       pinRelayDcDcSlow_m;
};

#endif  // ecranPrincipal_h
