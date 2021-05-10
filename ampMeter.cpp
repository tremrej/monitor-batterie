// ==================================================================================
// @file ampMeter.cpp
//
// Define the class to manage an amp meter to monitor a 12 volt batterie
// ==================================================================================

#include "ampMeter.h"

AmpMeter::AmpMeter( int address
                  , float ampOffset)
    : address_m (address)
    , ampOffset_m (ampOffset)
{
    ina219_mp = new Adafruit_INA219(address);
}

AmpMeter::~AmpMeter()
{
}

bool AmpMeter::init(float shuntAmp, float rshunt)
{
    if (! ina219_mp->begin()) 
    {
        demo = true;
    }
    else
    {
        ina219_mp->setCalibration_16V_rShunt_maxAmp( rshunt, shuntAmp);
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
    unsigned long timeSpent = 0;

    bool dataReady = false;
    float volt = ina219_mp->getBusVoltage_V(&dataReady);    // volts
    float curr = ina219_mp->getCurrent_mA();      // milli Amp
    float watt = ina219_mp->getPower_mW();        // milli Watt

    if (!demo)
    {
        if (dataReady)
        {
            cumulBusVolt_m    += volt;
            cumulCurrent_m    += curr;
            cumulPower_m      += watt;
        }
        else
        {
            // Returns 0 to indicate we read old data.
            return 0;
        }
        timeSpent = micros() - start;
    }
    else
#ifdef ARDUINO_AVR_MEGA2560
    {
        float demoVolt = 12.05;
        float demoCurrent = -2000.0;    // mA

        if (address_m == 0x40)   // Starter batterie
        {
            demoVolt = 0.0;
            demoCurrent = 0.0;    // mA
        }
        if (address_m == 0x41)   // House batterie
        {
            demoVolt = 0.0;
            demoCurrent = 0.0;    // mA
        }
        if (address_m == 0x44)   // Alternator
        {
            demoVolt = 0.0;
            demoCurrent = 0.0;    // mA
        }

        if (address_m == 0x45)   // Solar charger
        {
            demoVolt = 0.0;
            demoCurrent = 0.0;    // mA
        }
        cumulBusVolt_m    += demoVolt;
        cumulCurrent_m    += demoCurrent;
        cumulPower_m      += (demoVolt * demoCurrent);
        // On the NRF52 processor, the time in demo mode is less than one micro second. We make it 1 to not block the processing.
        timeSpent = 1;
    }
#else
    {
        float demoVolt = 12.05;
        float demoCurrent = -2000.0;    // mA

        if (address_m == 0x40)   // Starter batterie
        {
            demoVolt = 12.6;
            demoCurrent = -1000.0;    // mA
        }
        if (address_m == 0x41)   // House batterie
        {
            demoVolt = 13.0;
            demoCurrent = -5000.0;    // mA
        }
        if (address_m == 0x44)   // Alternator
        {
            demoVolt = 14.0;
            demoCurrent = 10000.0;    // mA
        }

        if (address_m == 0x45)   // Solar charger
        {
            demoVolt = 0.0;
            demoCurrent = 0.0;    // mA
        }
        cumulBusVolt_m    += demoVolt;
        cumulCurrent_m    += demoCurrent;
        cumulPower_m      += (demoVolt * demoCurrent);
        // On the NRF52 processor, the time in demo mode is less than one micro second. We make it 1 to not block the processing.
        timeSpent = 1;
    }
#endif

    avgCnt_m++;

    return timeSpent;
}

unsigned long AmpMeter::average()
{
    unsigned long tempTimestamp = micros();

    avgDuration_m = tempTimestamp - startAvg_m;
    startAvg_m = tempTimestamp;

    avgBusVolt_m = cumulBusVolt_m / avgCnt_m;
    avgCurrent_m = (cumulCurrent_m / avgCnt_m) + ampOffset_m;    // mA
    avgPower_m   = cumulPower_m   / avgCnt_m;    // mW
    // Clamp current to zero if within deadzone
    if (fabs(avgCurrent_m) < 10)
    {
        avgCurrent_m = 0.0;      // current deadzone of 10 ma
        avgPower_m   = 0.0;
    }
    ampSecondLastAvg_m = avgCurrent_m / 1000.0 * avgDuration_m;   // AmpMicroSecond
    ampSecondSinceReset_m += ampSecondLastAvg_m;

    cumulBusVolt_m = 0.0;
    cumulCurrent_m = 0.0;
    cumulPower_m   = 0.0;
    avgCnt_m       = 0;

    return avgDuration_m;
}

float AmpMeter::getBusVolt()
{

    bool dataReady = false;
    return ina219_mp->getBusVoltage_V(&dataReady);
}
float AmpMeter::getAvgBusVolt()
{
    return avgBusVolt_m;
}

float AmpMeter::getAvgCurrent()
{
    return (avgCurrent_m / 1000.0);   // Amp
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

