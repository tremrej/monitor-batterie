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
                              , AmpMeter &ampMeterAlternator
                              , Persistent &persistent
                              , int pinIgnition
                              , int pinRelayDcDcEnable
                              , int pinRelayDcDcSlow)
    : ampMeterStarter_m(&ampMeterStarter)
    , ampMeterHouse_m  (&ampMeterHouse)
    , ampMeterAlternator_m  (&ampMeterAlternator)
    , persistent_m (&persistent)
    , pinIgnition_m    (pinIgnition)
    , pinRelayDcDcEnable_m (pinRelayDcDcEnable)
    , pinRelayDcDcSlow_m   (pinRelayDcDcSlow)
    , holdOffTimerId_m(-1)
    , ignitionOn_m (false)
    , alternatorOn_m (false)
    , selectorBothOn_m (false)
    , voltageStartOfCharge_m (0.0)

{
    // Definition of states
    stateIdle_m       = new State(NULL, NULL, NULL);
    stateIgnitionOn_m = new State(NULL, NULL, NULL);
    stateAlternatorOn_m = new State(NULL, NULL, NULL);
    stateChargerEnabled_m = new State(NULL, NULL, NULL);

    // Creation of the finite state machine
    fsm_m = new Fsm(stateIdle_m);

    // Definition of the possible transition
    fsm_m->add_transition( stateIdle_m,           stateIgnitionOn_m,     ignitionTurnedOn_c,             &stopCharger);
    fsm_m->add_transition( stateIdle_m,           stateAlternatorOn_m,   ignitionTurnedOnAlterOn_c,      &startHoldoffTimer);
    fsm_m->add_transition( stateIgnitionOn_m,     stateIdle_m,           ignitionTurnedOff_c,            &stopCharger);
    fsm_m->add_transition( stateIgnitionOn_m,     stateIdle_m,           batterieSelectorAllDetected_c,  &stopCharger);
    fsm_m->add_transition( stateAlternatorOn_m,   stateIdle_m,           ignitionTurnedOff_c,            &stopCharger);
    fsm_m->add_transition( stateAlternatorOn_m,   stateIdle_m,           batterieSelectorAllDetected_c,  &stopCharger);
    fsm_m->add_transition( stateIgnitionOn_m,     stateAlternatorOn_m,   alternatorTurnedOn_c,           &startHoldoffTimer);
    fsm_m->add_transition( stateAlternatorOn_m,   stateChargerEnabled_m, chargerHoldoffExpired_c,        &startCharger);
    fsm_m->add_transition( stateChargerEnabled_m, stateIdle_m,           alternatorTurnedOff_c,          &stopCharger);
    //fsm_m->add_transition( stateChargerEnabled_m, stateIdle_m,           batterieSelectorAllDetected_c,  &stopCharger);
    fsm_m->add_transition( stateChargerEnabled_m, stateIdle_m,           ignitionTurnedOff_c,            &stopCharger);

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
    if (digitalRead(pinIgnition_m) == HIGH && !selectorBothOn_m)
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
    float alternatorCurrent = ampMeterAlternator_m->getAvgCurrent();
    if (alternatorCurrent > 0.5)     // TODO Use a configurable value
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
    else
    {
        if (alternatorOn_m)
        {
            fsm_m->trigger(alternatorTurnedOff_c);
            Serial.println("Alternator turned off");
            alternatorOn_m = false;
            delay(10);
        }
    }

    // Detection of battery selector to both (all, 1+2).
    // TODO Refine the charge voltage slope detection.
    if (fabs(ampMeterStarter_m->getAvgBusVolt() - ampMeterHouse_m->getAvgBusVolt()) < persistent_m->getAllDeadZone())
//         (ampMeterHouse_m->getAvgBusVolt() - voltageStartOfCharge_m < 0.3))    // Charge voltage slope detection
    {
        // Selector on Both
        fsm_m->trigger(batterieSelectorAllDetected_c);
        if (!selectorBothOn_m) Serial.println("Selector on Both");
        selectorBothOn_m = true;
    }
    else
    {
        // Selector not on Both
        //fsm_m->trigger(batterieSelectorNotOnAllDetected_c);
        if (selectorBothOn_m) Serial.println("Selector not on Both");
        selectorBothOn_m = false;
    }
}

void ChargerControl::startHoldoffTimer( )
{
    if (cc_g->holdOffTimerId_m == -1)
    {
        cc_g->holdOffTimerId_m = cc_g->sTimerEngine_m.setTimeout(cc_g->persistent_m->getDelay()*1000, holdoffTimerExpired);
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
    // save voltage of charging batterie, i.e. house in order to calculate slope of charge
    cc_g->voltageStartOfCharge_m = cc_g->ampMeterHouse_m->getAvgBusVolt();
    digitalWrite(cc_g->pinRelayDcDcEnable_m, HIGH);
    Serial.println("Start charger");
}
