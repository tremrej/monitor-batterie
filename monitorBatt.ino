/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_STMPE610.h>     // Touch screen
#include "Fonts/FreeMono9pt7b.h"
#include "ampMeter.h"
#include "floatPicker.h"
#include "chargerControl.h"
#include "ILI9341_util.h"   // printFloatAt(), getTouchXY()

// Platform. Uncomment only one.
//#define NRF52   // No need to uncomment this line when target board is NRF52. Cool.
//#define UNO     // Uncomment to compile for Arduino UNO
//#define ESP8266

#ifdef UNO
#include <TimerOne.h>
#endif
#ifdef NRF52
#include "NRF52TimerInterrupt.h"
#endif



#ifdef UNO
    // For the Adafruit shield, these are the default.
    #define TFT_DC 9
    #define TFT_CS 10
    // Touch screen chip select
    #define STMPE_CS 8
#endif
#ifdef ESP8266
    // For the Adafruit shield, these are the default.
    #define TFT_DC 15
    #define TFT_CS 0
    // Touch screen chip select
    #define STMPE_CS 16
    #define SD_CS 2
#endif
#ifdef NRF52
    #define TFT_DC 11
    #define TFT_CS 31
    // Touch screen chip select
    #define STMPE_CS 30
    #define SD_CS 27
#endif

#ifdef UNO
#define dimPin 3
#define pinIgnition A3
#define pinDcDcEnabled 2       // relay 1
#define pinDcDcSlow    5       // relay 2
#endif
#ifdef NRF52
#define dimPin 3
#define pinIgnition A3
#define pinDcDcEnabled 7       // relay 1
#define pinDcDcSlow    15      // relay 2
#endif

// The number of reading to average is fine tune in order to make sure we always read new data.
// We use the "conversion ready" bit from the INA219.
#define nbAvg 15

AmpMeter ampMeterStarter_g(0x40);
AmpMeter ampMeterHouse_g  (0x41);   // Bridge A0

ChargerControl chargerControl_g(ampMeterStarter_g, ampMeterHouse_g,pinIgnition, pinDcDcEnabled, pinDcDcSlow);

//ChargeControler chargeControler_gg(ampMeterStarter_g, ampMeterHouse_g, pinIgnition, pinDcDcEnabled, pinDcDcSlow);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

FloatPicker dcDcInVoltThresPicker_g = FloatPicker (tft, (char *) "DcDcInVoltThres", 11.0, 13.0, 0.01);


#ifdef ESP8266
  Ticker measurementTicker;
#endif
#ifdef NRF52
  NRF52Timer ITimer(NRF_TIMER_1);
#endif


// Touch screen calibration data for the raw touch data to the screen coordinates
#define TS_MINX 200
#define TS_MINY 330
#define TS_MAXX 3800
#define TS_MAXY 3750

Adafruit_GFX_Button configButton = Adafruit_GFX_Button();
#define configButtonX 50
#define configButtonY 210
#define configButtonW 80
#define configButtonH 31

Adafruit_GFX_Button resetButton = Adafruit_GFX_Button();
#define resetButtonX 200
#define resetButtonY 200
#define resetButtonW 40
#define resetButtonH 40

Adafruit_GFX_Button enableDcDcButton = Adafruit_GFX_Button();
#define enableDcDcButtonX 278
#define enableDcDcButtonY 170-20
#define enableDcDcButtonW 40
#define enableDcDcButtonH 31

Adafruit_GFX_Button backButton = Adafruit_GFX_Button();
#define backButtonX 320-62
#define backButtonY 210
#define backButtonW 60
#define backButtonH 31

Adafruit_GFX_Button dimButton = Adafruit_GFX_Button();
#define dimButtonX 2
#define dimButtonY 210
#define dimButtonW 40
#define dimButtonH 31

enum ActiveWindow_e {
    windowEcran1_c,
    windowConfig_c,
    windowPickDcDcInVoltThres_c,
    windowPickDcDcDelay_c,
    windowPickAllSelectDeadzone_c,
};

ActiveWindow_e activeWindow_g = windowEcran1_c;
ActiveWindow_e nextWindow_g   = windowEcran1_c;

// Backlight dim level 0..255
unsigned int dimLevel_g = 64;

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

  delay(500);

  // Create the buttons
  resetButton.initButton( &tft, resetButtonX, resetButtonY, resetButtonW, resetButtonH
                        , ILI9341_DARKGREY  // outline
                        , rgbTo565(128, 128, 128)
                        , rgbTo565(255, 255, 0)
                        //, ILI9341_GREEN  // fill
                        //, ILI9341_BLUE   // text
                        , (char *)"rst", 1, 2);
  enableDcDcButton.initButtonUL( &tft, enableDcDcButtonX, enableDcDcButtonY
                                   , enableDcDcButtonW, enableDcDcButtonH
                        , ILI9341_DARKGREY  // outline
                        , 0xC958  // fill
                        , 0x79EE   // text
                        , (char *)"off", 1, 2);
  configButton.initButtonUL( &tft, configButtonX, configButtonY, configButtonW, configButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_YELLOW  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"config  ", 1, 2);

  backButton.initButtonUL( &tft, backButtonX, backButtonY, backButtonW, backButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_ORANGE  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"back  ", 1, 2);
  dimButton.initButtonUL( &tft, dimButtonX, dimButtonY, dimButtonW, dimButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_DARKGREY  // fill
                        , ILI9341_BLACK   // text
                        , (char *)"dim", 1, 2);
  configButton.press(false);

  if (! ampMeterStarter_g.init()) 
  {
    Serial.println("Failed to find INA219 chip");
    tft.setCursor(5, 20);
    tft.println("Failed to find INA219 chip");
    tft.println("Enter demo mode");
    delay(1000);
  }
  else
  {
      ampMeterStarter_g.start();
  }
  if (! ampMeterHouse_g.init()) 
  {
    Serial.println("Failed to find INA219 chip");
    tft.setCursor(5, 20);
    tft.println("Failed to find INA219 chip");
    tft.println("Enter demo mode");
    delay(1000);
  }
  else
  {
      ampMeterHouse_g.start();
  }

  dcDcInVoltThresPicker_g.init(12.0);

  // Setup measurement timer
#ifdef UNO
  Timer1.initialize(2000000/nbAvg); // micro second
  Timer1.attachInterrupt(setMeasurementFlag);
#endif
#ifdef NRF52
  if (ITimer.attachInterruptInterval(2000000/nbAvg, setMeasurementFlag))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
  {
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
  }
#endif
#ifdef ESP8266
  measurementTicker.attach(2.0/nbAvg, setMeasurementFlag);
#endif
// Note: for M4 board, use SAMD interrupt by including the file SAMDTimerInterrupt.h
// Note: for NRF52 board use NRF52TimerInterrupt.h. See example /home/rejean/sketchbook/libraries/NRF52_TimerInterrupt/examples/TimerInterruptLEDDemo

  Serial.println(F("Done!"));;

  displayStaticEcran1();

  // Setup the backlight PWM control
  pinMode(dimPin, OUTPUT);
  analogWrite(dimPin, dimLevel_g);

  pinMode(pinDcDcEnabled, OUTPUT);
  digitalWrite(pinDcDcEnabled, LOW);
  pinMode(pinDcDcSlow, OUTPUT);
  digitalWrite(pinDcDcSlow, HIGH);
  pinMode(pinIgnition, INPUT);

  chargerControl_g.init();
}

void processChangeOfWindow(ActiveWindow_e window)
{
    switch (window)
    {
        case windowEcran1_c:
        {
            displayStaticEcran1();
            displayDataEcran1(0);
            break;
        }
        case windowConfig_c:
        {
            //displayStaticEcranConfig();
            dcDcInVoltThresPicker_g.drawStatic();
            break;
        }
        default: break;
    }
}

void displayStaticEcran1()
{
        tft.fillScreen(ILI9341_BLACK);
        tft.drawRect(0,0,tft.width(),tft.height(), rgbTo565(155,155,155));
        tft.drawFastHLine(0,18,tft.width(), rgbTo565(155,155,155));
        tft.setTextSize(1);
        tft.setCursor(5, 15);
        tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
        tft.println("Moniteur de batterie, v0.0");

        tft.setCursor(5, tft.getCursorY());
        tft.println("Volt:           V");
        tft.setCursor(5, tft.getCursorY());
        tft.println("Curr:           A");
        tft.setCursor(5, tft.getCursorY());
        tft.println("Pwr :           W");
        tft.setCursor(5, tft.getCursorY());
        tft.println("AH  :           AH");

        tft.setCursor(5, tft.getCursorY());
        tft.println("deltaT:         us");
        tft.setCursor(5, tft.getCursorY());
        tft.println("Time:            d h:m:s");

        resetButton.drawButton(false);
        enableDcDcButton.drawButton(enableDcDcButton.isPressed());
        configButton.drawButton(false);
        dimButton.drawButton(false);
        tft.setTextSize(1);
}

void displayDataEcran1(unsigned long deltaTAvg)
{
    printFloatAt(ampMeterStarter_g.getAvgBusVolt(), 1, 70, 33);
    printFloatAt(ampMeterStarter_g.getAvgCurrent(), 1, 70, 51);
    printFloatAt(ampMeterStarter_g.getAvgPower(), 1, 70, 69);
    printFloatAt(ampMeterStarter_g.getAmpHour(), 1, 70, 87);
    printIntAt(deltaTAvg, 1, 80, 105);
    printTimeFromMilliSec(millis() - ampMeterStarter_g.getTimeSinceReset(), 70, 123);
}

void displayStaticEcranConfig()
{
    tft.fillScreen(ILI9341_YELLOW);
    tft.setCursor(40, 170);
    tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(1);
    tft.print("Starter charge House");
//    configButton.drawButton(false);
    backButton.drawButton(false);
    dimButton.drawButton(false);
    enableDcDcButton.changeLabel(enableDcDcButton.isPressed()?(char *)"on":(char *)"off");
    enableDcDcButton.drawButton(enableDcDcButton.isPressed());
}

void takeMeasurementAndDisplay(bool display)
{
    static int loopCnt = 0;
    unsigned long deltaTTick = 0;
    unsigned long deltaTAvg = 0;

    deltaTTick = ampMeterStarter_g.tick();   // Take a measurement
    if (deltaTTick == 0)
    {
        Serial.print("Data not ready from INA219, time (ms): ");
        Serial.println(millis());
    }

    if (deltaTTick && loopCnt % nbAvg == 0)
    {
        deltaTAvg = ampMeterStarter_g.average();  // Calculate average since last average.
        //if (activeWindow_g == windowEcran1_c)
        if (display)
        {
            displayDataEcran1(deltaTAvg);
        }
        loopCnt = 0;
    }

    if (deltaTTick)
    {
        // Increment the loop count only if we got valid (new) date from INA219.
        loopCnt++;
    }
}

void checkUIEcran1()
{
    static bool toggle_s = false;
    static bool toggle2_s = false;
    int16_t x = 0;
    int16_t y = 0;
    if (getTouchXY(&x, &y))
    {
        if (resetButton.contains(x,y))
        {
            toggle_s = toggle_s? false: true;
            ampMeterStarter_g.resetAmpHour();
            resetButton.drawButton(toggle_s);
            delay(100);
        }
        if (configButton.contains(x,y))
        {
            toggle2_s = toggle2_s? false: true;
            configButton.press(toggle2_s);
            configButton.drawButton(toggle2_s);
            nextWindow_g = windowConfig_c;
            delay(100);
        }
        if (dimButton.contains(x,y))
        {
            adjustBacklight();
            delay(100);
        }
    }
}

void adjustBacklight()
{
    char newLabel[10];
    uint16_t temp;
    if (dimLevel_g > 64)
    {
        dimLevel_g -= 64;
    }
    else if (dimLevel_g > 20)
    {
        dimLevel_g = 0;
    }
    else
    {
        // Rollover to full power
        dimLevel_g = 250;
    }
    analogWrite(dimPin, dimLevel_g);
    temp = dimLevel_g*100/255;
    sprintf(newLabel, "%d", temp);
    // Add '%' to the new lable
    temp = strlen(newLabel);
    newLabel[temp] = '%';
    newLabel[temp+1] = '\0';
    dimButton.changeLabel(newLabel);
    dimButton.drawButton(false);
}

void checkUIConfig()
{
    static bool toggle_s = false;
    int16_t x = 0;
    int16_t y = 0;
    if (getTouchXY(&x, &y))
    {
        if (backButton.contains(x,y))
        {
            toggle_s = toggle_s? false: true;
            backButton.drawButton(toggle_s);
            nextWindow_g = windowEcran1_c;
            delay(100);
        }
        if (dimButton.contains(x,y))
        {
            adjustBacklight();
            delay(100);
        }
        if (enableDcDcButton.contains(x,y))
        {
            enableDcDcButton.press(!enableDcDcButton.isPressed());
            enableDcDcButton.changeLabel(enableDcDcButton.isPressed()?(char *)"on":(char *)"off");
            enableDcDcButton.drawButton(enableDcDcButton.isPressed());
            delay(100);
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

    chargerControl_g.tick();

    if (takeMeasurement_g)
    {
        takeMeasurement_g = false;    // Wait for next timer interrupt
        takeMeasurementAndDisplay(activeWindow_g == windowEcran1_c);
        Serial.print("Active window: ");
        Serial.print(activeWindow_g);
        Serial.println(millis());
    }

    if (nextWindow_g != activeWindow_g)
    {
        // We're changing window.
        processChangeOfWindow(nextWindow_g);
        activeWindow_g = nextWindow_g;
    }

    switch(activeWindow_g)
    {
        case windowEcran1_c: checkUIEcran1(); break;
        //case windowConfig_c: checkUIConfig(); break;
        case windowConfig_c:
        {
            if (dcDcInVoltThresPicker_g.checkUI())
            {
                // Value saved. Let's go back to the main window.
                nextWindow_g = windowEcran1_c;
            }
            break;
        }
        default: break;
    }
//    if (digitalRead(pinIgnition) == HIGH)
//    {
//        if (ignitionIsOff)
//        {
//            Serial.println("Ignition turned on");
//            ignitionIsOff = false;
//            // Debounce
//            delay(10);
//        }
//    }
//    else
//    {
//        if (!ignitionIsOff)
//        {
//            Serial.println("Ignition turned off");
//            ignitionIsOff = true;
//            delay(10);
//        }
//    }
}
