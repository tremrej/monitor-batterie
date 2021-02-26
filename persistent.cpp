// ==================================================================================
// @file persistent.cpp
//
// Define the class to controler the DcDc charger
// ==================================================================================

#include "persistent.h"

// Definition of class member

Persistent::Persistent()
    : version_m(0)
    , inputVoltThreshold_m(12.0)
{
}

Persistent::~Persistent( )
{
}

void Persistent::init( )
{
    EEPROM.setMemPool(0, EEPROMSizeMega);
    EEPROM.setMaxAllowedWrites(250);       // Maximum number of write allowed since restart. To prevent run away write.

    version_m.restore();
    if (version_m == CURRENTVERSION)
    {
        Serial.print("Persistent memory version: "); Serial.println(version_m);
        inputVoltThreshold_m.restore();
    }
    else
    {
        factoryReset();
    }
}

void Persistent::factoryReset( )
{
    Serial.println("Factory reset");
    version_m = 1;
    version_m.save();

    inputVoltThreshold_m = 11.25;
    inputVoltThreshold_m.save();
}

