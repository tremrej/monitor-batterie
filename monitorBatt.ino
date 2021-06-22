/***************************************************
  Monitor of two bank of 12 volts battery 
  plus DC-DC charge controller

Pour compiler:
Mega:
Type de carte: Arduino Mega or Mega 2560

Adafruit Feather:
Type de carte: Adafruit Feather nRF52832


  Written by Rejean Tremblay 
 ****************************************************/

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_STMPE610.h>     // Touch screen
#include "Fonts/FreeMono9pt7b.h"
#include "ampMeter.h"
#include "ecranPrincipal.h"
#include "ecranWatermark.h"
#include "ecranConfig.h"
#include "floatPicker.h"
#include "radioButton.h"
#include "chargerControl.h"
#include "ILI9341_util.h"   // printFloatAt(), getTouchXY()
#include "persistent.h"
#include "monitorBatt.h"    // ActiveWindow_e
//#include "Adafruit_LittleFS.h"  // Source ~/.arduino15/packages/adafruit/hardware/nrf52/0.21.0/libraries/Adafruit_LittleFS/src
//#include "EEPROMex.h"
//#include "EEPROMVar.h"

// Board supported
// =================================================
//      Macro                Name of board in IDE
// --------------------    -------------------------
// NRF52                   Adafruit Feather nRF52832
// ARDUINO_AVR_MEGA2560    Arduino Mega 2560
//
// Note: The code is too large for Arduino UNO

#ifdef ARDUINO_AVR_MEGA2560
#include <TimerOne.h>
#elif NRF52
#include "NRF52TimerInterrupt.h"
#endif

// Pins for Adafruit Arduino TFT shield
#ifdef ARDUINO_AVR_MEGA2560
#define TFT_DC 9
#define TFT_CS 10
#define STMPE_CS 8
#define dimPin 3
#elif NRF52
#define TFT_DC 11
#define TFT_CS 31
#define STMPE_CS 30
#define SD_CS 27
#define dimPin 3
#endif

// Pins for input/output
#ifdef ARDUINO_AVR_MEGA2560
#define pinIgnition A3
#define pinDcDcEnabled 5       // relay 1
#define pinDcDcSlow    2       // relay 2
#define blinkingLed 13
#elif NRF52
#define pinIgnition A3
#define pinDcDcEnabled 7       // relay 1
#define pinDcDcSlow    15      // relay 2
#define blinkingLed 17
#endif

// The number of reading to average is fine tune in order to make sure we always read new data.
// We use the "conversion ready" bit from the INA219.
//#define nbAvg 14
#define nbAvg 6
//#define nbAvg 3

// Average period in micro seconds
//#define averagePeriod 2000000
#define averagePeriod   1000000
//#define averagePeriod 600000

// Somehow the INA219 doesn't indicate zero when there is no current. I apply a offset if need be.
//                             address offsetCurrent
//                             ------- -------------
AmpMeter ampMeterStarter_g     (0x40, +000.0);
AmpMeter ampMeterHouse_g       (0x41, -048.5);   // Bridge A0
AmpMeter ampMeterAlternator_g  (0x44, -095.0);   // Bridge A1
AmpMeter ampMeterSolar_g       (0x45,    0.0);   // Bridge A0 & A1

Persistent persistent_g = Persistent();


//ChargeControler chargeControler_gg(ampMeterStarter_g, ampMeterHouse_g, pinIgnition, pinDcDcEnabled, pinDcDcSlow);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

char *charModeChoice[] = { (char *) "auto"
                         , (char *) "autoSlow"
                         , (char *) "disabled"
                         };

FloatPicker dcDcInVoltThresPicker_g = FloatPicker (tft, (char *) "Input volt threshold", 11.0, 13.0, 0.01);
FloatPicker dcDcInVoltThresToGoSlowPicker_g = FloatPicker (tft, (char *) "Slow input volt", 11.0, 13.0, 0.01);
FloatPicker dcDcInVoltHysteresisPicker_g = FloatPicker (tft, (char *) "Volt hysteresis", 0.1, 3.0, 0.01);
FloatPicker chargeStartDelay_g      = FloatPicker (tft, (char *) "Delai de demarrage", 5.0, 180.0, 1.0);
FloatPicker allDeadZone_g           = FloatPicker (tft, (char *) "ALl selector deadzone", 0.05, 2.0, 0.01);
RadioButton chargeMode_g            = RadioButton (tft, (char *) "Charge mode", charModeChoice, chargeModeMax_c);

ChargerControl chargerControl_g( ampMeterStarter_g
                               , ampMeterHouse_g
                               , ampMeterAlternator_g
                               , persistent_g
                               , chargeMode_g
                               , pinIgnition, pinDcDcEnabled, pinDcDcSlow);

EcranPrincipal ecranPrincipal_g = EcranPrincipal ( tft
                                                 , ampMeterStarter_g
                                                 , ampMeterHouse_g
                                                 , ampMeterAlternator_g
                                                 , ampMeterSolar_g
                                                 , chargerControl_g
                                                 , dimPin
                                                 , pinIgnition
                                                 , pinDcDcEnabled
                                                 , pinDcDcSlow);

EcranWatermark ecranWatermark_g = EcranWatermark ( tft
                                                 , ampMeterStarter_g
                                                 , ampMeterHouse_g
                                                 , ampMeterAlternator_g
                                                 , ampMeterSolar_g);

EcranConfig ecranConfig_g = EcranConfig( tft
                                       , persistent_g
                                       , dcDcInVoltThresPicker_g
                                       , dcDcInVoltThresToGoSlowPicker_g
                                       , dcDcInVoltHysteresisPicker_g
                                       , chargeStartDelay_g
                                       , allDeadZone_g);


#ifdef NRF52
  NRF52Timer ITimer(NRF_TIMER_1);
#endif

ActiveWindow_e activeWindow_g = windowEcran1_c;
ActiveWindow_e nextWindow_g   = windowEcran1_c;

bool ignitionIsOff = true;


bool takeMeasurement_g = false;

void setMeasurementFlag()
{
    takeMeasurement_g = true;
}

// ==================================================================================
// ==================================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("Moniteur de batterie, V0.0"); 

  persistent_g.init();
  dcDcInVoltThresPicker_g.init(persistent_g.getInputVoltThreshold());
  dcDcInVoltThresToGoSlowPicker_g.init(persistent_g.getInputVoltThresholdToGoSlow());
  dcDcInVoltHysteresisPicker_g.init(persistent_g.getVoltHysteresis());
  chargeStartDelay_g.init(persistent_g.getDelay());
  allDeadZone_g.init(persistent_g.getAllDeadZone());
  chargeMode_g.init(0);
 
  tft.begin();
  tft.setRotation(1);

  if (!ts.begin()) { 
    Serial.print("Unable to start touchscreen:");
    Serial.println(STMPE_CS);
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  
  Serial.print(F("Rounded rects (outline)  "));
  Serial.println(testRoundRects());
  tft.fillScreen(ILI9341_BLACK);

  tft.setFont(&FreeMono9pt7b);

  // Create the buttons

  if (! ampMeterStarter_g.init(200.0, 0.0005))     // shunt 200 Amp, 100 mv
  {
    Serial.println("Failed to find INA219 chip");
    tft.setCursor(5, 20);
    tft.println("Failed to find INA219 chip");
    tft.println("Enter demo mode");
    delay(100);
  }
  else
  {
      ampMeterStarter_g.start();
  }
  if (! ampMeterHouse_g.init(200.0, 0.0005))     // shunt 200 Amp, 100 mv
  {
    Serial.println("Failed to find INA219 chip");
    tft.setCursor(5, 20);
    tft.println("Failed to find INA219 chip");
    tft.println("Enter demo mode");
    delay(200);
  }
  else
  {
      ampMeterHouse_g.start();
  }
  if (! ampMeterAlternator_g.init(50.0, 0.001))     // Update with actual shunt from BleuSea (50 amp, 50 mv, R=0.001)
  {
    Serial.println("Failed to find INA219 chip");
    tft.setCursor(5, 20);
    tft.println("Failed to find INA219 chip");
    tft.println("Enter demo mode");
    delay(100);
  }
  else
  {
      ampMeterAlternator_g.start();
  }
  if (! ampMeterSolar_g.init(50.0, 0.0015))      // shunt 50 Amp, 75 mv
  {
    Serial.println("Failed to find INA219 chip");
    tft.setCursor(5, 20);
    tft.println("Failed to find INA219 chip");
    tft.println("Enter demo mode");
    delay(100);
  }
  else
  {
      ampMeterSolar_g.start();
  }

  ecranPrincipal_g.init();
  ecranWatermark_g.init();
  ecranConfig_g.init();

  // Setup measurement timer
#ifdef ARDUINO_AVR_MEGA2560
  Timer1.initialize(averagePeriod/nbAvg); // micro second
  Timer1.attachInterrupt(setMeasurementFlag);
#elif NRF52
  if (ITimer.attachInterruptInterval(averagePeriod/nbAvg, setMeasurementFlag))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
  {
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
  }
#endif
// Note: for M4 board, use SAMD interrupt by including the file SAMDTimerInterrupt.h

  Serial.println(F("Done!"));;

  ecranPrincipal_g.drawStatic();

  // Setup the backlight PWM control
  pinMode(dimPin, OUTPUT);
  analogWrite(dimPin, 128);

  pinMode(pinDcDcEnabled, OUTPUT);
  digitalWrite(pinDcDcEnabled, LOW);
  pinMode(pinDcDcSlow, OUTPUT);
  digitalWrite(pinDcDcSlow, LOW);
  pinMode(pinIgnition, INPUT);

  chargerControl_g.init();
}

void processChangeOfWindow(ActiveWindow_e window)
{
    switch (window)
    {
        case windowEcran1_c:
        {
            ecranPrincipal_g.drawStatic();
            ecranPrincipal_g.drawData();
            break;
        }
        case windowWatermark_c:
        {
            ecranWatermark_g.drawStatic();
            ecranWatermark_g.drawData();
            break;
        }
//        case windowPickDcDcInVoltThres_c:
//        {
//            dcDcInVoltThresPicker_g.drawStatic();
//            break;
//        }
//        case windowPickDcDcDelay_c:
//        {
//            chargeStartDelay_g.drawStatic();
//            break;
//        }
        case windowConfig_c:
        {
            ecranConfig_g.drawStatic();
            ecranConfig_g.drawData();
            break;
        }
        case windowChargeMode_c:
        {
            chargeMode_g.drawStatic();
            chargeMode_g.drawData();
            break;
        }
        default: break;
    }
}

void takeMeasurementAndDisplay(bool display)
{
    static int avgCnt = 0;
    unsigned long deltaTStarter = 0;

    deltaTStarter      = ampMeterStarter_g.tick();   // Take a measurement
                         ampMeterHouse_g.tick();   // Take a measurement
                         ampMeterAlternator_g.tick();   // Take a measurement
                         ampMeterSolar_g.tick();   // Take a measurement
    if (deltaTStarter == 0)
    {
        Serial.print("Data not ready from starter IA219, time (ms): ");
        Serial.println(millis());
    }
    else
    {
       // Increment the loop count only if we got valid (new) date from INA219.
       avgCnt++;
        if (avgCnt % nbAvg == 0)
        {
            ampMeterStarter_g.average();  // Calculate average since last average.
            ampMeterHouse_g.average();  // Calculate average since last average.
            ampMeterAlternator_g.average();  // Calculate average since last average.
            ampMeterSolar_g.average();  // Calculate average since last average.
            if (activeWindow_g == windowEcran1_c)
            {
                ecranPrincipal_g.drawData();
            }
            else if (activeWindow_g == windowWatermark_c)
            {
                ecranWatermark_g.drawData();
            }
            avgCnt = 0;

            // Toggle the led
            digitalWrite(blinkingLed, !digitalRead(blinkingLed));
        }
    }
}

unsigned long testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  w     = min(tft.width(), tft.height());
  start = micros();
  for(i=0; i<w; i+=6) {
    i2 = i / 2;
    tft.drawRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(i, 0, 0));
  }

  return micros() - start;
}

void loop(void) {


    if (takeMeasurement_g)
    {
        // It's time to take a measurement according to the timer interrupt.
        takeMeasurement_g = false;    // Wait for next timer interrupt
        takeMeasurementAndDisplay(activeWindow_g == windowEcran1_c);

        // Tick the charge controller.
        chargerControl_g.tick();
    }

    if (nextWindow_g != activeWindow_g)
    {
        // We're changing window.
        processChangeOfWindow(nextWindow_g);
        activeWindow_g = nextWindow_g;
    }

    switch(activeWindow_g)
    {
        case windowEcran1_c:
        {
            nextWindow_g = ecranPrincipal_g.checkUI();
            break;
        }
        case windowWatermark_c:
        {
            nextWindow_g = ecranWatermark_g.checkUI();
            break;
        }
        case windowConfig_c:
        {
            nextWindow_g = ecranConfig_g.checkUI();
            break;
        }
        case windowPickDcDcInVoltThres_c:
        {
            float ttt = dcDcInVoltThresPicker_g.getValue();
            // For now config is simply a input voltage limit picker.
            if (dcDcInVoltThresPicker_g.checkUI())
            {
                // Value saved. Let's go back to the main window.
                nextWindow_g = windowEcran1_c;
            }
            if (ttt != dcDcInVoltThresPicker_g.getValue())
            {
                persistent_g.setInputVoltThreshold(dcDcInVoltThresPicker_g.getValue());
            }
            break;
        }
        case windowChargeMode_c:
        {
            if ( chargeMode_g.checkUI())
            {
                nextWindow_g = windowEcran1_c;
            }
            break;
        }
        default: break;
    }
}
