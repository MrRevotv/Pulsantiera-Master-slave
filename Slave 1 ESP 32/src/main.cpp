#include <Arduino.h>
#include "Config.h"
#include "PowerManager.h"
#include "CommsManager.h"
#include "InputManager.h"
#include "EncoderManager.h"
#include "FeedbackManager.h"

struct_message myData;

void setup() {
    FeedbackManager::init();
    FeedbackManager::showBootScreen();
    FeedbackManager::showWelcomeMessage();

    PowerManager::init();
    InputManager::init();
    EncoderManager::init();

    CommsManager::init(MASTER_MAC_ADDRESS);
    myData.slaveID = SLAVE_ID;
}

void loop() {
    // --- FASE 1: GESTIONE ENERGIA E SPEGNIMENTO ---
    PowerManager::update();
    
    if (PowerManager::isTurningOff()) {
        if (PowerManager::getTurnOffCountdown() <= 0) {
            
            // 1. Mostra il messaggio a schermo
            FeedbackManager::showReleaseMsg();
            
            // 2. Blocca il sistema finché il dito è sul pulsante
            while(!digitalRead(POWER_BTN_PIN)) {
                delay(50);
            }
            
            // 3. Il dito è stato tolto. Spegni i pixel e vai a nanna.
            PowerManager::executeDeepSleep(); 
            
        } else {
            // Mostra il conto alla rovescia (ora partirà da 3)
            FeedbackManager::showPowerOffCountdown(PowerManager::getTurnOffCountdown());
        }
        return; // Salta il resto del loop finché siamo in fase di spegnimento
    }

    // --- FASE 2: LETTURA INPUT FISICI ---
    bool inputsChanged = InputManager::readAll();
    bool encodersChanged = EncoderManager::readAll();

    // --- FASE 3: IMPACCHETTAMENTO DATI ---
    myData.buttons = (InputManager::getButtonsState() & 0x000FFFFF) | EncoderManager::getButtonsState();
    myData.axis1 = EncoderManager::getAxis(1);
    myData.axis2 = EncoderManager::getAxis(2);

// --- FASE 4: AGGIORNAMENTO SCHERMO ---
    static unsigned long lastDisplayUpdate = 0; // Timer per lo schermo

    if (PowerManager::isScreenOn()) {
        String latestAction = "";
        if (inputsChanged) latestAction = InputManager::getLastActionText();
        
        // AGGIORNA LO SCHERMO SOLO SE:
        // 1. È stato premuto un tasto (inputsChanged == true)
        // 2. Sono passati almeno 250ms dall'ultimo aggiornamento (per l'animazione)
        if (inputsChanged || (millis() - lastDisplayUpdate > 250)) {
            FeedbackManager::updateHUD(
                PowerManager::getBatteryPercentage(), 
                CommsManager::isConnected(), 
                latestAction
            );
            lastDisplayUpdate = millis(); // Resetta il timer
        }
    } else {
        FeedbackManager::turnOffScreen();
    }

    // --- FASE 5: TRASMISSIONE ---
    CommsManager::sendIfReady(myData);
}