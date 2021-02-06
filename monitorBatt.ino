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


#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_STMPE610.h>     // Touch screen
#include "Fonts/FreeMono9pt7b.h"
#include "ampMeter.h"

#include <TimerOne.h>

//#include <Wire.h>


// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
// Touch screen chip select
#define STMPE_CS 8

// Dim backlight
#define dimPin 3

// The number of reading to average is fine tune in order to make sure we always read new data.
// We use the "conversion ready" bit from the INA219.
#define nbAvg 15

AmpMeter ampMeter_g;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);


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
};

ActiveWindow_e activeWindow_g = windowEcran1_c;
ActiveWindow_e nextWindow_g   = windowEcran1_c;

// Backlight dim level 0..255
unsigned int dimLevel_g = 64;


bool takeMeasurement_g = false;

void setMeasurementFlag()
{
    takeMeasurement_g = true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Moniteur de batterie, V0.0"); 
 
  tft.begin();
  tft.setRotation(1);

  if (!ts.begin()) { 
    Serial.println("Unable to start touchscreen.");
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
                        , ILI9341_GREEN  // fill
                        , ILI9341_BLUE   // text
                        , (char *)"rst", 1, 2);
  enableDcDcButton.initButtonUL( &tft, enableDcDcButtonX, enableDcDcButtonY
                                   , enableDcDcButtonW, enableDcDcButtonH
                        , ILI9341_DARKGREY  // outline
                        , ILI9341_DARKGREY  // fill
                        , ILI9341_RED   // text
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

  if (! ampMeter_g.init()) 
  {
    Serial.println("Failed to find INA219 chip");
    tft.setCursor(5, 20);
    tft.println("Failed to find INA219 chip");
    tft.println("Enter demo mode");
    delay(1000);
//    while (1) { delay(10); }
  }
  else
  {
      ampMeter_g.start();
  }

  // Setup measurement timer
  Timer1.initialize(2000000/nbAvg); // micro second
  Timer1.attachInterrupt(setMeasurementFlag);

  Serial.println(F("Done!"));;

  displayStaticEcran1();

  // Setup the backlight PWM control
  pinMode(dimPin, OUTPUT);
  analogWrite(dimPin, dimLevel_g);
}

void printTimeFromMilliSec(unsigned long milliSec, int x, int y)
{
    // format: d hh:mm:ss

    float ttt;
    int ti;

    tft.fillRect(x-4, y-12, 116, 20, ILI9341_BLUE);
    tft.setCursor(x,y);

    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);

    // days
    ttt = milliSec/(1000L*60*60*24);
    tft.print(ti); tft.print(" ");

    // hours
    ttt = milliSec/(1000L*60*60);
    ti = (int)trunc(ttt)%24;
    if (ti < 10) tft.print("0");
    tft.print(ti); tft.print(":");

    // minutes
    ttt = milliSec/(1000L*60);
    ti = (int)trunc(ttt)%60;
    if (ti < 10) tft.print("0");
    tft.print(ti); tft.print(":");

    // seconds
    //ttt = milliSec/(1000);
    //ti = (int)trunc(ttt)%60;
    ti = (milliSec%60000)/1000;
    if (ti < 10) tft.print("0");
    if (ti < 0) ti = 0;
    tft.print(ti);
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
            displayStaticEcranConfig();
            break;
        }
        default: break;
    }
}

void displayStaticEcran1()
{
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextSize(1);
        tft.setCursor(0, 12);
        tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
        tft.println("Moniteur de batterie, v0.0");

        tft.println("Volt:           V");
        tft.println("Curr:           A");
        tft.println("Pwr :           W");
        tft.println("AH  :           AH");

        tft.println("deltaT:         us");
        tft.println("Time:            d h:m:s");

        resetButton.drawButton(false);
        enableDcDcButton.drawButton(enableDcDcButton.isPressed());
        configButton.drawButton(false);
        dimButton.drawButton(false);
        tft.setTextSize(1);
}

void displayDataEcran1(unsigned long deltaTAvg)
{
    printFloatAt(ampMeter_g.getAvgBusVolt(), 7, 65, 30);
    printFloatAt(ampMeter_g.getAvgCurrent(), 7, 65, 48);
    printFloatAt(ampMeter_g.getAvgPower(), 7, 65, 66);
    printFloatAt(ampMeter_g.getAmpHour(), 7, 65, 84);
    printIntAt(deltaTAvg, 7, 75, 102);
    printTimeFromMilliSec(millis() - ampMeter_g.getTimeSinceReset(), 65, 120);
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
    enableDcDcButton.changeLabel(enableDcDcButton.isPressed()?"on":"off");
    enableDcDcButton.drawButton(enableDcDcButton.isPressed());
}

void printFloatAt(float value, int width, int x, int y)
{
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);
    tft.fillRect(x, y-12, 100, 20, ILI9341_RED);
    char ttt[64];

    // Add minus sign when the integer part is zero.
    if (value < 0.0 && value > -1.0)
    {
        tft.setCursor(x+15,y);
        tft.print("-");
    }
    sprintf(ttt, "%4d.%02d", (int) trunc(value), (int) abs(trunc(((value-trunc(value))*100))));
    tft.setCursor(x,y); tft.print(ttt);
}

void printIntAt(unsigned long value, int width, int x, int y)
{
    tft.setTextSize(1);
    tft.fillRect(x-4, y-12, 100, 20, ILI9341_RED);
    tft.setCursor(x,y); tft.print(value);
}

void takeMeasurementAndDisplay(bool display)
{
    static int loopCnt = 0;
    unsigned long deltaTTick = 0;
    unsigned long deltaTAvg = 0;

    deltaTTick = ampMeter_g.tick();   // Take a measurement
    if (deltaTTick == 0)
    {
        Serial.print("Data not ready from INA219, time (ms): ");
        Serial.println(millis());
    }

    if (deltaTTick && loopCnt % nbAvg == 0)
    {
        deltaTAvg = ampMeter_g.average();  // Calculate average since last average.
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

bool getTouchXY(int16_t *x, int16_t *y)
{
    if (ts.touched())
    {
        
        ts.writeRegister8(STMPE_FIFO_STA, 0);    // unreset the fifo
        if (!ts.bufferEmpty())
        {
            // Retrieve a point  
            TS_Point p = ts.getPoint(); 
            // Scale using the calibration #'s
            // and rotate coordinate system
            p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
            p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
            *y = tft.height() - p.x;
            *x = p.y;
            ts.writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints, in this example unneeded depending in use
            ts.writeRegister8(STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
            return true;
        }
        ts.writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints, in this example unneeded depending in use
    }

    return false;
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
            ampMeter_g.resetAmpHour();
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
        dimLevel_g = 8;
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
    static bool toggle2_s = false;
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
            enableDcDcButton.changeLabel(enableDcDcButton.isPressed()?"on":"off");
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

    if (takeMeasurement_g)
    {
        takeMeasurement_g = false;    // Wait for next timer interrupt
        takeMeasurementAndDisplay(activeWindow_g == windowEcran1_c);
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
        case windowConfig_c: checkUIConfig(); break;
        default: break;
    }
}
