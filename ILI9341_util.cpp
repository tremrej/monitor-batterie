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


#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_STMPE610.h>     // Touch screen

// Touch screen calibration data for the raw touch data to the screen coordinates
#ifndef ESP8266
// UNO
#define TS_MINX 200
#define TS_MINY 330
#define TS_MAXX 3800
#define TS_MAXY 3750
#else
// Feather 
#define TS_MINX 100
#define TS_MINY 3750
#define TS_MAXX 3800
#define TS_MAXY 100
#endif

// External declaration
extern Adafruit_ILI9341 tft;
extern Adafruit_STMPE610 ts;


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

void printFloatAt(float value, int textSizeY, int x, int y)
{
    tft.setTextSize(1,textSizeY);
    tft.setTextColor(ILI9341_WHITE);
    tft.fillRect(x, y-14-((textSizeY-1)*10), 100, 20+((textSizeY-1)*10), ILI9341_RED);
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

// Taken from http://www.barth-dev.de/online/rgb565-color-picker
uint16_t rgbTo565(uint8_t red, uint8_t green, uint8_t blue)
{
    return (((red & 0xf8)<<8) + ((green & 0xfc)<<3)+(blue>>3));
}

