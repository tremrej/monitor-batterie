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
    , delay_m(10.0)
    , allDeadZone_m(0.3)
    , efficiencyOfCharge_m(90)
    , capacityStarter_m(60)
    , capacityHouse_m(75)
    , fullChargeVoltStarter_m(14.0)
    , fullChargeVoltHouse_m(14.0)
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
        delay_m.restore();
        allDeadZone_m.restore();
        efficiencyOfCharge_m.restore();
        capacityStarter_m.restore();
        capacityHouse_m.restore();
        fullChargeVoltStarter_m.restore();
        fullChargeVoltHouse_m.restore();
    }
    else
    {
        factoryReset();
    }
}

void Persistent::factoryReset( )
{
    Serial.println("Factory reset");
    version_m = CURRENTVERSION;
    version_m.save();

    inputVoltThreshold_m = 11.25;
    inputVoltThreshold_m.save();
    delay_m = 10;
    delay_m.save();
    allDeadZone_m = 0.3;
    allDeadZone_m.save();
    efficiencyOfCharge_m = 90;   // %
    efficiencyOfCharge_m.save();
    capacityStarter_m = 60;   // AH
    capacityStarter_m.save();
    capacityHouse_m = 75;   // AH
    capacityHouse_m.save();
    fullChargeVoltStarter_m = 14.0;
    fullChargeVoltStarter_m.save();
    fullChargeVoltHouse_m = 14.0;
    fullChargeVoltHouse_m.save();
}

