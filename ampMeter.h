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

    AmpMeter(int address = 0x40);
    ~AmpMeter();

    // Return true if success, false failure
    bool init();

    // Start the timer for before the first tick.
    void start();

    // Retrieve measurement from the amp meter and add them up (for averaging)
    // @return time spend, micro second
    unsigned long tick();

    // Average the measurements since the last average or start.
    // @return time spend, micro second
    unsigned long average();

    void resetAmpHour();

    float getAvgBusVolt();
    float getAvgCurrent();
    float getAvgPower();
    float getAmpSecondLastAvg();

    float getAmpHour();

    unsigned long getTimeSinceReset();

private:

    int address_m;
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

    float ampSecondSinceReset_m;
};

#endif  // ampMeter
