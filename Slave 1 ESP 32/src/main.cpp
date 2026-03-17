#include <Arduino.h>
#include "Config.h"
#include "PowerManager.h"
#include "CommsManager.h"
#include "InputManager.h"
#include "EncoderManager.h"
#include "FeedbackManager.h"

// Crea il pacchetto dati globale usando la struttura definita in Config.h
struct_message myData;

void setup() {
    // 1. Inizializza l'hardware visivo per dare subito un feedback all'utente
    FeedbackManager::init();
    FeedbackManager::showBootScreen();

    // 2. Inizializza i sensori e la logica
    PowerManager::init();
    InputManager::init();
    EncoderManager::init();

    // 3. Inizializza la comunicazione radio verso il Master
    CommsManager::init(MASTER_MAC_ADDRESS);

    // 4. Assegna l'ID a questa scheda
    myData.slaveID = SLAVE_ID;
}

void loop() {
    // --- FASE 1: GESTIONE ENERGIA ---
    PowerManager::update();
    
    if (PowerManager::isTurningOff()) {
        FeedbackManager::showPowerOffCountdown(PowerManager::getTurnOffCountdown());
        return; // Blocca tutto il resto finché il tasto è premuto
    }

    // --- FASE 2: LETTURA INPUT FISICI ---
    bool inputsChanged = InputManager::readAll();
    bool encodersChanged = EncoderManager::readAll();

    // --- FASE 3: IMPACCHETTAMENTO DATI ---
    // Unisce i bit della matrice (0-15) e i click encoder (16-19) con le rotazioni (28-31)
    myData.buttons = InputManager::getButtonsState() | EncoderManager::getButtonsState();
    
    // Assegna i due potenziometri virtuali
    myData.axis1 = EncoderManager::getAxis(1);
    myData.axis2 = EncoderManager::getAxis(2);

    // --- FASE 4: AGGIORNAMENTO SCHERMO ---
    if (PowerManager::isScreenOn()) {
        String latestAction = "";
        if (inputsChanged) latestAction = InputManager::getLastActionText();
        
        FeedbackManager::updateHUD(
            PowerManager::getBatteryPercentage(), 
            CommsManager::isConnected(), 
            latestAction
        );
    } else {
        FeedbackManager::turnOffScreen();
    }

    // --- FASE 5: TRASMISSIONE ---
    // La funzione si occupa da sola di rispettare il limite di 20ms
    CommsManager::sendIfReady(myData);
}