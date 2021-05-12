// ==================================================================================
// @file ampMeter.h
//
// Declare a class to manage an amp meter to monitor a 12 volt batterie
// ==================================================================================
#ifndef ampMeter_h
#define ampMeter_h

#include <Adafruit_INA219.h>

// ==================================================================================
//  @class AmpMeter
// 
// The output of the amp meter INA219 is quite noisy. This class imlemente a filter in the form
// of a simple averaging.
// This class also calculate the energy.
// ==================================================================================
class AmpMeter
{
public:

    AmpMeter( int address = 0x40
            , float ampOffset = 0.0);    // amp offset (amp)
    ~AmpMeter();

    // Return true if success, false failure
    bool init(float shuntAmp, float rshunt);

    // Start the timer for before the first tick.
    void start();

    // Retrieve measurement from the amp meter and add them up (for averaging)
    // @return time spend, micro second
    unsigned long tick();

    // Average the measurements since the last average or start.
    // @return time spend, micro second
    unsigned long average();

    void resetAmpHour();

    unsigned long getTimeSinceReset();

    float getBusVolt();        // Volt
    float getAvgBusVolt();     // Volt
    float getAvgCurrent();     // Amp
    float getAvgPower();       // Watt
    float getAmpSecondLastAvg();

    float getAmpHour();

    bool getBusVoltWatermark(float *high, float*low);
    bool getCurrentWatermark(float *high, float*low);
    void resetWatermark();


private:

    int address_m;
    float ampOffset_m;     // Units: milli amp
    bool demo;

    Adafruit_INA219 *ina219_mp;  // Driver of the amp meter

    unsigned long startAvg_m;    // Timestamp, millisecond
    unsigned long timestampResetMicro_m;    // Timestamp, microsecond, will roll over in 70 minutes.
    unsigned long timestampResetMilli_m;    // Timestamp, millisecond, will roll over in 50 days.
    int           avgCnt_m;

    float cumulBusVolt_m;    // Volt
    float cumulCurrent_m;    // mA
    float cumulPower_m;      // mW

    float avgDuration_m;         // millisecond
    float avgBusVolt_m;          // Volt
    float avgCurrent_m;          // Amp
    float avgPower_m;            // Watt
    float ampSecondLastAvg_m;    // AmpSecond

    float voltMin_m;             // Volt
    float voltMax_m;             // Volt
    float ampMin_m;              // Amp
    float ampMax_m;              // Amp

    float ampSecondSinceReset_m;
};

#endif  // ampMeter
