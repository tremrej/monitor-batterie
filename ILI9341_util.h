// ==================================================================================
// @file ILI9341_util.h
//
// Utility function related to the tft display 9341 from Adafruit
// ==================================================================================
#ifndef ili9341_util_h
#define ili9341_util_h

#include "Adafruit_ILI9341.h"

void printTimeFromMilliSec(unsigned long milliSec, int x, int y);

void printFloatAt(float value, int width, int x, int y);

void printIntAt(unsigned long value, int width, int x, int y);

bool getTouchXY(int16_t *x, int16_t *y);

// Convert a 24 bits RGB color into a 16 bits RGB color, aka 565 color
uint16_t rgbTo565(uint8_t red, uint8_t green, uint8_t blue);

#endif  // ili9341_util_h
