#pragma once
#include <Arduino.h>

// --- PINOUT HARDWARE MASTER ---
#define ENC1_CLK 4
#define ENC1_DT 5
#define ENC1_SW 6

#define ENC2_CLK 7
#define ENC2_DT 10
#define ENC2_SW 11

#define BUZZER_PIN 2 // Pin assegnato al Buzzer

const int NUM_BUTTONS = 16;
const int buttonPins[NUM_BUTTONS] = {12, 13, 14, 15, 16, 17, 18, 21, 38, 39, 40, 41, 42, 47, 48, 45};

// --- STRUTTURE DATI ESP-NOW ---
typedef struct struct_message {
    uint8_t slaveID;
    uint32_t buttons;
    int16_t axis1;
    int16_t axis2;
} struct_message;

// --- STATO GLOBALE DEL SISTEMA ---
// Questa struttura permette a Display, Input e Comms di "parlarsi"
struct SystemState {
    bool isDebugMode = false;
    String lastActionText = ""; // Lasciato vuoto per la logica di stand-by
    unsigned long lastActionTime = 0;
    
    long enc1Count = 0;
    long enc2Count = 0;
    int pressedButtonsCount = 0; // Utile per la debug screen
    
    // --- STATO SLAVE 1 ---
    bool slave1Connected = false;
    unsigned long lastSlave1Msg = 0;
    int16_t slave1Axis1 = 0;

    // --- STATO SLAVE 2 ---
    bool slave2Connected = false;
    unsigned long lastSlave2Msg = 0;
    int16_t slave2Axis1 = 0;

    // --- STATO SLAVE 3 ---
    bool slave3Connected = false;
    unsigned long lastSlave3Msg = 0;
    int16_t slave3Axis1 = 0;

    // --- STATO BUZZER ---
    bool triggerConnectBeep = false;    // Richiede il doppio bip
    bool triggerDisconnectBeep = false; // Richiede il bip lungo
};

// Dichiariamo che queste variabili esistono nel main.cpp
extern SystemState sysState;
extern struct_message incomingData;