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
    , alternatorOn_m (false)

{
    // Definition of states
    stateIdle_m       = new State(NULL, NULL, NULL);
    stateIgnitionOn_m = new State(NULL, NULL, NULL);
    stateAlternatorOn_m = new State(NULL, NULL, NULL);
    stateChargerEnabled_m = new State(NULL, NULL, NULL);

    // Creation of the finite state machine
    fsm_m = new Fsm(stateIdle_m);

    // Definition of the possible transition
    fsm_m->add_transition( stateIdle_m,           stateIgnitionOn_m,     ignitionTurnedOn_c,      NULL);
    fsm_m->add_transition( stateIdle_m,           stateAlternatorOn_m,   ignitionTurnedOnAlterOn_c, &startHoldoffTimer);
    fsm_m->add_transition( stateIgnitionOn_m,     stateIdle_m,           ignitionTurnedOff_c,     &stopCharger);
    fsm_m->add_transition( stateAlternatorOn_m,   stateIdle_m,           ignitionTurnedOff_c,     &stopCharger);
    fsm_m->add_transition( stateIgnitionOn_m,     stateAlternatorOn_m,   alternatorTurnedOn_c,    &startHoldoffTimer);
    fsm_m->add_transition( stateAlternatorOn_m,   stateChargerEnabled_m, chargerHoldoffExpired_c, &startCharger);
    fsm_m->add_transition( stateIgnitionOn_m,     stateChargerEnabled_m, chargerHoldoffExpired_c, &startCharger);
    fsm_m->add_transition( stateChargerEnabled_m, stateIdle_m,           alternatorTurnedOff_c,   &stopCharger);
    fsm_m->add_transition( stateChargerEnabled_m, stateIdle_m,           ignitionTurnedOff_c,     &stopCharger);

    // Set the pointer to the singleton charge controller.
    cc_g = this;
}

ChargerControl::~ChargerControl( )
{
    free (stateIdle_m);
}

bool ChargerControl::init( )
{
    // Start the state machine.
    // Then from now on the state machine is event driven.
    fsm_m->run_machine();
    return true;
}

void ChargerControl::tick( )
{
    // Kick the timer engine
    sTimerEngine_m.run();

    // Check ignition key
    if (digitalRead(pinIgnition_m) == HIGH)
    {
        if (!ignitionOn_m)
        {
            if (!alternatorOn_m)
            {
                fsm_m->trigger(ignitionTurnedOn_c);
            }
            else
            {
                // The alternator is already on for some reason
                fsm_m->trigger(ignitionTurnedOnAlterOn_c);
            }
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

    // Check state of alternator
    float starterBattCurrent = ampMeterStarter_m->getAvgCurrent();
    if (starterBattCurrent < -1.0)     // TODO Use a configurable value
    {
        //if (!alternatorOn_m || fsm_m->getCurrentState() != stateAlternatorOn_m)
        if (!alternatorOn_m)
        {
            fsm_m->trigger(alternatorTurnedOn_c);
            Serial.println("Alternator turned on");
            alternatorOn_m = true;
            // Debounce
            delay(10);
        }
    }
    else if (starterBattCurrent > 10.0)   // TODO Use a configurable value
    {
        // When the DC-DC charger kicks in, the starter battery will most likely discharge for a short period of time.
        // I need to think of an algorithm to detect reliably that the alternator stopped. The most reliable way
        // would be to measure the shunt on the alternator output. I might do that. According to spec from BlueSea it's a 50 Amp 50 mV.
        // I just need to put a INA219 on it.
        // For now I'll declare the alternator off by a big discharge rate.
        if (alternatorOn_m)
        {
            fsm_m->trigger(alternatorTurnedOff_c);
            Serial.println("Alternator turned off");
            alternatorOn_m = false;
            delay(10);
        }
    }
}

void ChargerControl::checkAlternator()
{
    if (cc_g->ampMeterStarter_m->getAvgCurrent() < -1.0)
    {
        Serial.println("Alternator is already on");
        cc_g->fsm_m->trigger(alternatorTurnedOn_c);
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
