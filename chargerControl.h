// ==================================================================================
// @file chargerControl.h
//
// Declare a class to control the DC-DC charger
// ==================================================================================
#ifndef chargerControl_h
#define chargerControl_h

#include "Fsm.h"          // Source: ~/sketchbook/librarie/arduino-fsm
#include "SimpleTimer.h"  // Source: ~/sketchbook/librarie/SimpleTimer
#include "ampMeter.h"

// ==================================================================================
//  @class ChargerControl
// 
// This is a singleton class.
// ==================================================================================
class ChargerControl
{
public:

    enum State_e {
        idle_c,
        ignitionOn_c,
        alternatorOn_c,
        chargerOn_c,
        ignitionOff_c
    };

    enum Event_e {
        ignitionTurnedOn_c,
        ignitionTurnedOnAlterOn_c,
        ignitionTurnedOff_c,
        alternatorTurnedOn_c,
        alternatorTurnedOff_c,
        chargerHoldoffExpired_c,
        batterieSelectorAllDetected_c,
        batterieSelectorNotOnAllDetected_c,
    };

    ChargerControl( AmpMeter &ampMeterStarter
                  , AmpMeter &ampMeterHouse
                  , int pinIgnition
                  , int pinRelayDcDcEnable
                  , int pinRelayDcDcSlow);
    ~ChargerControl();

    // Return true if success, false failure
    bool init();

    void tick();

private:

    static void startHoldoffTimer();
    static void holdoffTimerExpired();
    static void stopCharger();
    static void startCharger();

    AmpMeter *ampMeterStarter_m;
    AmpMeter *ampMeterHouse_m;
    int       pinIgnition_m;
    int       pinRelayDcDcEnable_m;
    int       pinRelayDcDcSlow_m;

    // List of state
    State *stateIdle_m;
    State *stateIgnitionOn_m;
    State *stateAlternatorOn_m;
    State *stateChargerEnabled_m;

    Fsm *fsm_m;

    SimpleTimer sTimerEngine_m;
    int         holdOffTimerId_m;

    // Local state
    bool ignitionOn_m;
    bool alternatorOn_m;
    bool selectorBothOn_m;

    float voltageStartOfCharge_m;

};

extern ChargerControl *cc_g;

#endif  // ChargerControl
