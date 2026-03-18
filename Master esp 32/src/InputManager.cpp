#include "InputManager.h"
#include <ESP32Encoder.h>
#include "Config.h"
#include "SimGamepad.h"

extern SimGamepad Gamepad;

ESP32Encoder encoder1;
ESP32Encoder encoder2;
bool buttonStates[NUM_BUTTONS] = {false};
unsigned long encoderButtonReleaseTime[129] = {0};
bool debugComboFired = false;

// Questa funzione sostituisce la logica del PowerManager dello Slave:
// Aggiorna il testo e "sveglia" l'HUD sul display registrando il tempo.
void triggerAction(String text) {
    sysState.lastActionText = text;
    sysState.lastActionTime = millis();
}

void initInputs() {
    ESP32Encoder::useInternalWeakPullResistors = UP;
    encoder1.attachHalfQuad(ENC1_DT, ENC1_CLK);
    encoder2.attachHalfQuad(ENC2_DT, ENC2_CLK);
    
    pinMode(ENC1_SW, INPUT_PULLUP);
    pinMode(ENC2_SW, INPUT_PULLUP);

    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }
}

void readInputs() {
    // 1. Lettura Encoders (Testo aggiornato in maiuscolo)
    long newEnc1 = encoder1.getCount() / 2;
    if (newEnc1 > sysState.enc1Count) { Gamepad.setButton(1, true); encoderButtonReleaseTime[1] = millis() + 50; triggerAction("ENC 1 (DX)"); }
    if (newEnc1 < sysState.enc1Count) { Gamepad.setButton(2, true); encoderButtonReleaseTime[2] = millis() + 50; triggerAction("ENC 1 (SX)"); }
    sysState.enc1Count = newEnc1;

    long newEnc2 = encoder2.getCount() / 2;
    if (newEnc2 > sysState.enc2Count) { Gamepad.setButton(4, true); encoderButtonReleaseTime[4] = millis() + 50; triggerAction("ENC 2 (DX)"); }
    if (newEnc2 < sysState.enc2Count) { Gamepad.setButton(5, true); encoderButtonReleaseTime[5] = millis() + 50; triggerAction("ENC 2 (SX)"); }
    sysState.enc2Count = newEnc2;

    // Spegnimento finti bottoni encoder dopo 50ms
    for(int i=1; i<=128; i++) {
        if(encoderButtonReleaseTime[i] > 0 && millis() > encoderButtonReleaseTime[i]) {
            Gamepad.setButton(i, false);
            encoderButtonReleaseTime[i] = 0;
        }
    }

    // 2. Lettura Pulsanti Encoder (Testo aggiornato stile Slave)
    bool enc1SwPressed = !digitalRead(ENC1_SW);
    bool enc2SwPressed = !digitalRead(ENC2_SW);
    Gamepad.setButton(3, enc1SwPressed);
    Gamepad.setButton(6, enc2SwPressed);
    
    static bool prevE1 = false, prevE2 = false;
    if(enc1SwPressed && !prevE1) triggerAction("ENC 1 (CLICK)"); prevE1 = enc1SwPressed;
    if(enc2SwPressed && !prevE2) triggerAction("ENC 2 (CLICK)"); prevE2 = enc2SwPressed;

    // 3. Lettura 16 Pulsanti Fisici (Testo aggiornato stile Slave)
    sysState.pressedButtonsCount = 0; // Reset contatore per debug
    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool currentState = !digitalRead(buttonPins[i]);
        if (currentState) sysState.pressedButtonsCount++; 
        
        if (currentState != buttonStates[i]) {
            buttonStates[i] = currentState;
            Gamepad.setButton(7 + i, currentState);
            // Genera il testo "TASTO X" tutto in maiuscolo
            if (currentState) triggerAction("TASTO " + String(i + 1)); 
        }
    }

    // 4. CHECK COMBO SEGRETA (Encoder 1 SW + Tasto Fisico 1)
    if (enc1SwPressed && buttonStates[0]) {
        if (!debugComboFired) {
            sysState.isDebugMode = !sysState.isDebugMode;
            debugComboFired = true;
            triggerAction(sysState.isDebugMode ? "DEBUG ON" : "DEBUG OFF");
        }
    } else {
        debugComboFired = false;
    }
}