For different reason, be it to enable compile and to add features, here are the changes that I've made to some libraries:

/home/rejean/sketchbook/libraries/Adafruit_INA219:
- Change ins219_currentDivider_mA from int to float in order to allow CurrentLSB higher than 1 mA.
- Added setCalibration_16V_50A_75mv()

/home/rejean/sketchbook/libraries/Adafruit_BusIO/Adafruit_SPIDevice.cpp:
- In order to compile for Feather ESP8266 I had adapt Adafruit_SPIDevice::transfer(). For Feather, the macro STM32 seems to not be defined but the function _spi->tranfer(buffer, len) is not defined.
