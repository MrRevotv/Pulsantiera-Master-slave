#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. IMPOSTAZIONI DI RETE (ESP-NOW)
// ==========================================
// Indirizzo MAC del Master ESP32-S3
const uint8_t MASTER_MAC_ADDRESS[] = {0xDC, 0xB4, 0xD9, 0x38, 0x20, 0x54};

// Struttura dati per la trasmissione (Deve essere identica sul Master!)
typedef struct struct_message {
    uint8_t slaveID;
    uint32_t buttons;
    int16_t axis1;
    int16_t axis2;
} struct_message;

// ==========================================
// 2. PIN DI SISTEMA E ALIMENTAZIONE
// ==========================================
#define SLAVE_ID          1
#define BATTERY_PIN       35
#define POWER_BTN_PIN     15 

// ==========================================
// 3. TIMERS E COSTANTI DI SISTEMA (in millisecondi)
// ==========================================
#define SCREEN_TIMEOUT    1200000UL // 20 minuti prima di spegnere l'OLED
#define SLEEP_TIMEOUT     3600000UL // 60 minuti prima dello spegnimento totale
#define ENC_HOLD_MS       60        // Durata del "click" virtuale della rotazione per Virpil
#define ESPNOW_SEND_RATE  20        // Intervallo di trasmissione radio (evita di intasare il WiFi)

// ==========================================
// 4. CONFIGURAZIONE MATRICE 4x4
// ==========================================
const byte MATRIX_ROWS = 4; 
const byte MATRIX_COLS = 4; 
// I pin fisici della matrice
const byte ROW_PINS[MATRIX_ROWS] = {32, 33, 25, 14}; 
const byte COL_PINS[MATRIX_COLS] = {13, 12, 4, 2}; 

// ==========================================
// 5. CONFIGURAZIONE ENCODER E POTENZIOMETRI
// ==========================================
// Encoder 1 (Standard)
#define ENC1_PIN_A        5
#define ENC1_PIN_B        18
#define ENC1_SW           34

// Encoder 2 (Standard)
#define ENC2_PIN_A        19
#define ENC2_PIN_B        21
#define ENC2_SW           36

// Encoder 3 (Convertito in Potenziometro - Asse 1)
#define ENC3_PIN_A        22
#define ENC3_PIN_B        23
#define ENC3_SW           39

// Encoder 4 (Convertito in Potenziometro - Asse 2)
#define ENC4_PIN_A        1
#define ENC4_PIN_B        3
#define ENC4_SW           0

#endif // CONFIG_H