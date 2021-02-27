// ==================================================================================
// @file ecranConfig.h
//
// Declare a class to manage the config window.
// ==================================================================================
#ifndef ecranConfig_h
#define ecranConfig_h

#include "Adafruit_GFX.h"    // Adafruit_GFX_Button
#include "monitorBatt.h"     // ActiveWindow_e
#include "floatPicker.h"
#include "persistent.h"

// ==================================================================================
//  @class EcranPrincipal
// 
// ==================================================================================
class EcranConfig
{
public:

    EcranConfig( Adafruit_GFX &tft
               , Persistent   &persistent
               , FloatPicker  &inputVoltThreshold
               , FloatPicker  &chargeStartDelay
               , FloatPicker  &allDeadZone);
    ~EcranConfig();

    void init();

    void processChangeOfWindow(ActiveWindow_e window);

    // Check the UI.
    // @retval next window
    ActiveWindow_e checkUI();

    void drawStatic();
    void drawData();

private:
    Adafruit_GFX *tft_m;
    Persistent   *persistent_m;
    FloatPicker  *inVoltThreshold_m;
    FloatPicker  *chargeStartDelay_m;
    FloatPicker  *allDeadZone_m;

    Adafruit_GFX_Button inVoltButton_m;
    Adafruit_GFX_Button delayButton_m;
    Adafruit_GFX_Button allDeadZoneButton_m;
    Adafruit_GFX_Button backButton_m;

    ActiveWindow_e activeWindow_m;
    ActiveWindow_e nextWindow_m;

};

#endif  // ecranConfig_h
