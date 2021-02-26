// ==================================================================================
// @file chargerControl.h
//
// Declare a class manage persistent storage of config data
// ==================================================================================
#ifndef persistent_h
#define persistent_h

#include "EEPROMex.h"
#include "EEPROMVar.h"

#define CURRENTVERSION 1      // Version and inputVoltThreshold

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

    void  setInputVoltThreshold(float volt) { inputVoltThreshold_m = volt; inputVoltThreshold_m.save(); };
    float getInputVoltThreshold()           { return inputVoltThreshold_m; };

private:

    EEPROMVar<byte>  version_m;
    EEPROMVar<float> inputVoltThreshold_m;

};

#endif  // persisten_h
