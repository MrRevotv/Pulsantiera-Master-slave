#include "BuzzerManager.h"
#include <Arduino.h>
#include "Config.h"

unsigned long buzzerTimer = 0;
int buzzerState = 0; 
// Stati: 0=Spento, 1=Bip Lungo, 2=DoppioBip Fase 1, 3=Pausa, 4=DoppioBip Fase 2

void initBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void handleBuzzer() {
    unsigned long currentMillis = millis();

    // 1. Controllo dei flag di sistema per avviare una sequenza
    if (sysState.triggerConnectBeep) {
        sysState.triggerConnectBeep = false;
        buzzerState = 2; // Avvia doppio bip
        buzzerTimer = currentMillis;
        digitalWrite(BUZZER_PIN, HIGH);
    } 
    else if (sysState.triggerDisconnectBeep) {
        sysState.triggerDisconnectBeep = false;
        buzzerState = 1; // Avvia bip lungo
        buzzerTimer = currentMillis;
        digitalWrite(BUZZER_PIN, HIGH);
    }

    // 2. Macchina a stati per gestire la durata dei suoni
    if (buzzerState == 1) { 
        // Bip lungo (2000ms)
        if (currentMillis - buzzerTimer >= 2000) {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 0;
        }
    } 
    else if (buzzerState == 2) { 
        // Doppio bip: Primo Suono (100ms)
        if (currentMillis - buzzerTimer >= 100) {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 3;
            buzzerTimer = currentMillis;
        }
    } 
    else if (buzzerState == 3) { 
        // Doppio bip: Pausa (100ms)
        if (currentMillis - buzzerTimer >= 100) {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzerState = 4;
            buzzerTimer = currentMillis;
        }
    } 
    else if (buzzerState == 4) { 
        // Doppio bip: Secondo Suono (100ms)
        if (currentMillis - buzzerTimer >= 100) {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerState = 0;
        }
    }
}