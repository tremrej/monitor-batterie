// ==================================================================================
// @file ampMeter.cpp
//
// Define the class to manage an amp meter to monitor a 12 volt batterie
// ==================================================================================

#include "ampMeter.h"

AmpMeter::AmpMeter(int address)
{
    ina219_mp = new Adafruit_INA219(address);
}

AmpMeter::~AmpMeter()
{
}

bool AmpMeter::init()
{
    if (! ina219_mp->begin()) 
    {
        demo = true;
    }
    else
    {
        ina219_mp->setCalibration_16V_50A_75mv();
        demo = false;
    }
    start();
    return !demo;
}

void AmpMeter::start()
{
    startAvg_m = micros();
//    timestampResetMicro_m = startAvg_m;
    timestampResetMilli_m = millis();
    avgCnt_m = 0;
    cumulBusVolt_m    = 0.0;
    cumulCurrent_m    = 0.0;
    cumulPower_m      = 0.0;
    avgBusVolt_m = 0.0;
    avgCurrent_m = 0.0;
    avgPower_m   = 0.0;
    ampSecondLastAvg_m = 0.0;
    ampSecondSinceReset_m = 0.0;
}

void AmpMeter::resetAmpHour()
{
    ampSecondSinceReset_m = 0.0;
//    timestampResetMicro_m = micros();
    timestampResetMilli_m = millis();
}

unsigned long AmpMeter::tick()
{
    unsigned long start = micros();
    if (!demo)
    {
        bool dataReady = false;
        cumulBusVolt_m    += ina219_mp->getBusVoltage_V(&dataReady);    // volts
        cumulCurrent_m    += ina219_mp->getCurrent_mA();      // milli Amp
        cumulPower_m      += ina219_mp->getPower_mW();        // milli Watt

        if (!dataReady)
        {
            // Returns 0 to indicate we read old data.
            return 0;
        }
    }
    else
    {
#define demoVolt 12.35
#define demoCurrent 60000
        cumulBusVolt_m    += demoVolt;
        cumulCurrent_m    += demoCurrent;
        cumulPower_m      += (demoVolt * demoCurrent);
    }
    avgCnt_m++;

    return 1;
}

unsigned long AmpMeter::average()
{
    unsigned long tempTimestamp = micros();

    avgDuration_m = tempTimestamp - startAvg_m;
    startAvg_m = tempTimestamp;

    avgBusVolt_m = cumulBusVolt_m / avgCnt_m;
    avgCurrent_m = cumulCurrent_m / avgCnt_m;    // mA
    avgPower_m   = cumulPower_m   / avgCnt_m;    // mW
    ampSecondLastAvg_m = avgCurrent_m / 1000.0 * avgDuration_m;   // AmpMicroSecond
    ampSecondSinceReset_m += ampSecondLastAvg_m;

    cumulBusVolt_m = 0.0;
    cumulCurrent_m = 0.0;
    cumulPower_m   = 0.0;
    avgCnt_m       = 0;

    return avgDuration_m;
}

float AmpMeter::getAvgBusVolt()
{
    return avgBusVolt_m;
}

float AmpMeter::getAvgCurrent()
{
    return avgCurrent_m / 1000;   // Amp
}

float AmpMeter::getAvgPower()
{
    return avgPower_m / 1000;   // Watt
}

float AmpMeter::getAmpSecondLastAvg()
{
    return ampSecondLastAvg_m;
}

float AmpMeter::getAmpHour()
{
    return ampSecondSinceReset_m / 3600000000.0;    // 3600000000 micro second per hour
}

unsigned long AmpMeter::getTimeSinceReset()
{
    return timestampResetMilli_m;
}

