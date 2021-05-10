// ==================================================================================
// @file chargerControl.cpp
//
// Define the class to controler the DcDc charger
// ==================================================================================

#include "chargerControl.h"
#include "monitorBatt.h"       // chargeModeAuto_c

// Definition of class member

ChargerControl *cc_g = NULL;

//int ChargerControl::holdOffTimerId_m = -1;
//SimpleTimer ChargerControl::sTimerEngine_m;

ChargerControl::ChargerControl( AmpMeter &ampMeterStarter
                              , AmpMeter &ampMeterHouse
                              , AmpMeter &ampMeterAlternator
                              , Persistent &persistent
                              , RadioButton &chargeMode
                              , int pinIgnition
                              , int pinRelayDcDcEnable
                              , int pinRelayDcDcSlow)
    : ampMeterStarter_m(&ampMeterStarter)
    , ampMeterHouse_m  (&ampMeterHouse)
    , ampMeterAlternator_m  (&ampMeterAlternator)
    , persistent_m (&persistent)
    , chargeMode_m (&chargeMode)
    , pinIgnition_m    (pinIgnition)
    , pinRelayDcDcEnable_m (pinRelayDcDcEnable)
    , pinRelayDcDcSlow_m   (pinRelayDcDcSlow)
    , holdOffTimerId_m(-1)
    , ignitionOn_m (false)
    , alternatorOn_m (false)
    , selectorBothOn_m (false)
    , selectorOnBothBeginTimestamp_m(0)
    , houseVoltageBeforeCharge_m (0.0)
    , chargeStartTime_m (0)
    , slowCharge_m(false)
    , forceSlowCharge_m(false)
    , slowChargeTimestamp_m(0)
    , fullChargeTimestamp_m(0)

{
    // Definition of states
    stateIdle_m       = new State(NULL, NULL, NULL);
    stateAlternatorOn_m = new State(NULL, NULL, NULL);
    stateChargerEnabled_m = new State(NULL, NULL, NULL);

    // Creation of the finite state machine
    fsm_m = new Fsm(stateIdle_m);

    // Definition of the possible transition
    fsm_m->add_transition( stateIdle_m,           stateAlternatorOn_m,   alternatorTurnedOn_c,           &startHoldoffTimer);
    fsm_m->add_transition( stateAlternatorOn_m,   stateIdle_m,           batterieSelectorAllDetected_c,  &stopCharger);
    fsm_m->add_transition( stateAlternatorOn_m,   stateChargerEnabled_m, chargerHoldoffExpired_c,        &startCharger);
    fsm_m->add_transition( stateChargerEnabled_m, stateIdle_m,           alternatorTurnedOff_c,          &stopCharger);

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

    if (chargeMode_m->getValue() == chargeModeDisabled_c)
    {
        if ( fsm_m->getCurrentState() == stateChargerEnabled_m)
        {
            fsm_m->trigger(alternatorTurnedOff_c);
        }
        // Nothing else todo
        return;
    }

    if (chargeMode_m->getValue() == chargeModeAutoSlow_c)
    {
        forceSlowCharge_m = true;
        if ( fsm_m->getCurrentState() != stateChargerEnabled_m)
        {
            // We trigger a transition to alternator off. If the alternator is on it will trigger a start charge transition.
            alternatorOn_m = false;
            fsm_m->trigger(alternatorTurnedOff_c);
        }
    }

    if (chargeMode_m->getValue() == chargeModeAuto_c)
    {
        forceSlowCharge_m = false;

        if ( fsm_m->getCurrentState() != stateChargerEnabled_m)
        {
            // We trigger a transition to alternator off. If the alternator is on it will trigger a start charge transition.
            alternatorOn_m = false;
            fsm_m->trigger(alternatorTurnedOff_c);
        }
    }


    // Check ignition key
    if (digitalRead(pinIgnition_m) == HIGH && !selectorBothOn_m)
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
        if (!ignitionOn_m)
        {
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
    if (alternatorCurrent > 1.0)     // TODO Use a configurable value
    {
        //if (!alternatorOn_m || fsm_m->getCurrentState() != stateAlternatorOn_m)
        if (!alternatorOn_m)
        {
            // Alternator just started
            houseVoltageBeforeCharge_m = ampMeterHouse_m->getAvgBusVolt();
            fsm_m->trigger(alternatorTurnedOn_c);
            Serial.println("Alternator turned on");
            alternatorOn_m = true;
            // Debounce
            delay(10);
        }
    }
    else
    {
        // Alternator is stopped
        if (alternatorOn_m)
        {
            // Alternator just stopped
            fsm_m->trigger(alternatorTurnedOff_c);
            Serial.println("Alternator turned off");
            alternatorOn_m = false;
            delay(10);
        }
    }

    // Detection of battery selector to both (all, 1+2).
    if (fabs(ampMeterStarter_m->getAvgBusVolt() - ampMeterHouse_m->getAvgBusVolt()) < persistent_m->getAllDeadZone()) 
    {
        if (selectorOnBothBeginTimestamp_m == 0)
        {
            selectorOnBothBeginTimestamp_m = millis();
        }
    }
    else
    {
        selectorOnBothBeginTimestamp_m = 0;
    }

    // Logic to detect batterie selector on position "both":
    // - Not charging and both bank "same" voltage
    // - Charging and "not fully charged" and "same" voltage for 10 seconds.
    // Note: we do not declare "both" if fully charged. That could be enhanced... I'll get back to it.
    if ( (fsm_m->getCurrentState() != stateChargerEnabled_m &&
          selectorOnBothBeginTimestamp_m != 0) ||
         (fsm_m->getCurrentState() == stateChargerEnabled_m &&
          fullChargeTimestamp_m == 0 &&
          (selectorOnBothBeginTimestamp_m != 0 && (millis() - selectorOnBothBeginTimestamp_m > (10 * 1000)))))    // Grace period of 10 seconds while charger is on
    {
        // Batterie Selector on Both
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

    // Set charge speed
    if (( fsm_m->getCurrentState() == stateChargerEnabled_m &&
         ampMeterStarter_m->getAvgBusVolt() < persistent_m->getInputVoltThresholdToGoSlow()) ||
         forceSlowCharge_m)
    {
        // Too hard on the source batterie. We go slow charge.
        if (!slowCharge_m)
        {
            setSlowCharge(true);
            slowChargeTimestamp_m = millis();
        }
        else
        {
            // Already slow charge.
            // Nothing to do
        }
    }
    else
    {
        // Check if we can go back to fast charge
        if (slowCharge_m)
        {
            if (ampMeterStarter_m->getAvgBusVolt() > (persistent_m->getInputVoltThresholdToGoSlow() + 
                                                      persistent_m->getVoltHysteresis()))
            {
                // The voltage of the source batterie is good now.
                //unsigned long ttt = 5L*60L*1000L;       // 5 minutes
                if (millis() - slowChargeTimestamp_m > 5L*60L*1000L)
                {
                    // And we have been on slow charge for at least x minutes.
                    // Back to fast charge
                    setSlowCharge(false);
                    slowChargeTimestamp_m = 0;
                }
            }
        }
    }

    // Detection of full charge
    if ( ( fsm_m->getCurrentState() == stateChargerEnabled_m &&
           ampMeterHouse_m->getAvgBusVolt() > persistent_m->getFullChargeVoltHouse()) ||
         ( fsm_m->getCurrentState() == stateChargerEnabled_m &&
           ampMeterHouse_m->getAvgBusVolt() > 13.4 &&
           ampMeterHouse_m->getAvgCurrent() > -5.0 ))     // Charging rate less than 5 amps.
    {
        if (fullChargeTimestamp_m == 0)
        {
            Serial.println("Fully charged");
        }

        fullChargeTimestamp_m = millis();
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
//    cc_g->houseVoltageBeforeCharge_m = cc_g->ampMeterHouse_m->getAvgBusVolt();
    cc_g->setSlowCharge(false);
    cc_g->fullChargeTimestamp_m = 0;
    digitalWrite(cc_g->pinRelayDcDcEnable_m, HIGH);
    Serial.println("Start charger");
}

void ChargerControl::setSlowCharge(bool slowCharge)
{
    slowCharge_m = slowCharge;
    digitalWrite(cc_g->pinRelayDcDcSlow_m, slowCharge? HIGH:LOW);
    Serial.print("Charge speed ");
    Serial.println(slowCharge? (char *) "slow":(char *) "fast");
}

bool ChargerControl::isChanging()
{
    return fsm_m->getCurrentState() == stateChargerEnabled_m;
}

bool ChargerControl::isChangingSlow()
{
    return fsm_m->getCurrentState() == stateChargerEnabled_m && slowCharge_m;
}

bool ChargerControl::batterySelectorOnBoth()
{
    return selectorBothOn_m;
}

