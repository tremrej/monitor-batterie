// ==================================================================================
// @file chargerControl.cpp
//
// Define the class to controler the DcDc charger
// ==================================================================================

#include "chargerControl.h"

// Definition of class member

ChargerControl *cc_g = NULL;

//int ChargerControl::holdOffTimerId_m = -1;
//SimpleTimer ChargerControl::sTimerEngine_m;

ChargerControl::ChargerControl( AmpMeter &ampMeterStarter
                              , AmpMeter &ampMeterHouse
                              , int pinIgnition
                              , int pinRelayDcDcEnable
                              , int pinRelayDcDcSlow)
    : ampMeterStarter_m(&ampMeterStarter)
    , ampMeterHouse_m  (&ampMeterHouse)
    , pinIgnition_m    (pinIgnition)
    , pinRelayDcDcEnable_m (pinRelayDcDcEnable)
    , pinRelayDcDcSlow_m   (pinRelayDcDcSlow)
    , holdOffTimerId_m(-1)
    , ignitionOn_m (false)

{
    stateIdle_m       = new State(NULL, NULL, NULL);
    stateIgnitionOn_m = new State(NULL, NULL, NULL);
    stateChargerEnabled_m = new State(NULL, NULL, NULL);

    fsm_m = new Fsm(stateIdle_m);

    fsm_m->add_transition( stateIdle_m,           stateIgnitionOn_m,     ignitionTurnedOn_c,      &startHoldoffTimer);
    fsm_m->add_transition( stateIgnitionOn_m,     stateIdle_m,           ignitionTurnedOff_c,     &stopCharger);
    fsm_m->add_transition( stateChargerEnabled_m, stateIdle_m,           ignitionTurnedOff_c,     &stopCharger);
    fsm_m->add_transition( stateIgnitionOn_m,     stateChargerEnabled_m, chargerHoldoffExpired_c, &startCharger);

    cc_g = this;
}

ChargerControl::~ChargerControl( )
{
}

bool ChargerControl::init( )
{
    fsm_m->run_machine();
    return true;
}

void ChargerControl::tick( )
{
    sTimerEngine_m.run();

    if (digitalRead(pinIgnition_m) == HIGH)
    {
        if (!ignitionOn_m)
        {
            fsm_m->trigger(ignitionTurnedOn_c);
            Serial.println("Ignition turned on");
            ignitionOn_m = true;
            // Debounce
            delay(10);
        }
    }
    else
    {
        if (ignitionOn_m)
        {
            fsm_m->trigger(ignitionTurnedOff_c);
            Serial.println("Ignition turned off");
            ignitionOn_m = false;
            delay(10);
        }
    }
}

void ChargerControl::startHoldoffTimer( )
{
    if (cc_g->holdOffTimerId_m == -1)
    {
        cc_g->holdOffTimerId_m = cc_g->sTimerEngine_m.setTimeout(5000, holdoffTimerExpired);
        Serial.println("Start holdoff timer");
    }
    else
    {
        Serial.println("holdoff timer already running");
    }
}

void ChargerControl::holdoffTimerExpired( )
{
    // Start charger
    cc_g->holdOffTimerId_m = -1;
    cc_g->fsm_m->trigger(chargerHoldoffExpired_c);
    Serial.println("Enabled charger Holdoff expired");
}

void ChargerControl::stopCharger( )
{
    // Stop charger
    int ttt = cc_g->holdOffTimerId_m;
    if (ttt >= 0)
    {
        cc_g->sTimerEngine_m.deleteTimer(ttt);
        cc_g->holdOffTimerId_m = -1;
    }
    digitalWrite(cc_g->pinRelayDcDcEnable_m, LOW);
    Serial.println("Stop charger");
}

void ChargerControl::startCharger( )
{
    // Start charger
    digitalWrite(cc_g->pinRelayDcDcEnable_m, HIGH);
    Serial.println("Start charger");
}
