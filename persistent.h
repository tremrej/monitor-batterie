// ==================================================================================
// @file chargerControl.h
//
// Declare a class manage persistent storage of config data
// ==================================================================================
#ifndef persistent_h
#define persistent_h

#include "EEPROMex.h"
#include "EEPROMVar.h"

#define CURRENTVERSION 4      // 

// ==================================================================================
//  @class Persistent
// 
// This is a singleton class.
// ==================================================================================
class Persistent
{
public:

    Persistent( );
    ~Persistent();

    void factoryReset();
    void setVersion(byte version) { version_m = version; version_m.save(); };
    byte getVersion()             { return version_m; };
    void init();

    void  setInputVoltThreshold(float value) { inputVoltThreshold_m = value; inputVoltThreshold_m.save(); };
    float getInputVoltThreshold()           { return inputVoltThreshold_m; };

    void  setDelay(float value) { delay_m = value; delay_m.save(); };
    float getDelay()            { return delay_m; };

    void  setAllDeadZone(float value) { allDeadZone_m = value; allDeadZone_m.save(); };
    float getAllDeadZone()            { return allDeadZone_m; };

    void  setEffOfCharge(float value) { efficiencyOfCharge_m = value; efficiencyOfCharge_m.save(); };
    float getEffOfCharge()            { return efficiencyOfCharge_m; };

    void  setCapacityStarter(float value) { capacityStarter_m = value; capacityStarter_m.save(); };
    float getCapacityStarter()            { return capacityStarter_m; };

    void  setCapacityHouse(float value) { capacityHouse_m = value; capacityHouse_m.save(); };
    float getCapacityHouse()            { return capacityHouse_m; };

    void  setFullChargeVoltStarter(float value) { fullChargeVoltStarter_m = value; fullChargeVoltStarter_m.save(); };
    float getFullChargeVoltStarter()            { return fullChargeVoltStarter_m; };

    void  setFullChargeVoltHouse(float value) { fullChargeVoltHouse_m = value; fullChargeVoltHouse_m.save(); };
    float getFullChargeVoltHouse()            { return fullChargeVoltHouse_m; };
private:

    EEPROMVar<byte>  version_m;
    EEPROMVar<float> inputVoltThreshold_m;
    EEPROMVar<float> delay_m;
    EEPROMVar<float> allDeadZone_m;
    EEPROMVar<float> efficiencyOfCharge_m;
    EEPROMVar<float> capacityStarter_m;
    EEPROMVar<float> capacityHouse_m;
    EEPROMVar<float> fullChargeVoltStarter_m;
    EEPROMVar<float> fullChargeVoltHouse_m;

};

#endif  // persistent_h
