// ==================================================================================
// @file monitorBatt.h
//
// General declaration
// ==================================================================================
#ifndef monitorBatt_h
#define monitorBatt_h

enum ActiveWindow_e {
    windowEcran1_c,
    windowWatermark_c,
    windowMotor_c,
    windowConfig_c,
    windowPickDcDcInVoltThres_c,
    windowPickDcDcInVoltThresToGoSlow_c,
    windowPickDcDcInVoltHysteresis_c,
    windowPickDcDcDelay_c,
    windowPickAllSelectDeadzone_c,
    windowChargeMode_c,
};

enum ChargeMode_e {
    chargeModeAuto_c     = 0,
    chargeModeAutoSlow_c = 1,
    chargeModeDisabled_c = 2,
    chargeModeMax_c      = 3
};

#endif  // monitorBatt_h
