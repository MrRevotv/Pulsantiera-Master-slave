#include "EncoderManager.h"
#include "PowerManager.h"

namespace EncoderManager {

    // Inizializzazione degli oggetti CustomEncoder
    CustomEncoder encoder1(ENC1_PIN_A, ENC1_PIN_B, false);       
    CustomEncoder encoder2(ENC2_PIN_A, ENC2_PIN_B, false);      
    CustomEncoder encoder3_pot(ENC3_PIN_A, ENC3_PIN_B, true);  
    CustomEncoder encoder4_pot(ENC4_PIN_A, ENC4_PIN_B, true);  

    uint32_t currentEncButtons = 0;
    
    // Variabili per mantenere premuti gli encoder abbastanza a lungo
    unsigned long enc1_CW_Time = 0;
    unsigned long enc1_CCW_Time = 0;
    unsigned long enc2_CW_Time = 0;
    unsigned long enc2_CCW_Time = 0;

    // Variabili per lo smoothing degli assi
    float smoothAxis1 = 16383.0;
    float smoothAxis2 = 16383.0;

    void init() {
        encoder1.begin();
        encoder2.begin();
        encoder3_pot.begin();
        encoder4_pot.begin();
    }

    uint32_t getButtonsState() {
        return currentEncButtons;
    }

    int16_t getAxis(int axisNumber) {
        if (axisNumber == 1) return (int16_t)smoothAxis1;
        if (axisNumber == 2) return (int16_t)smoothAxis2;
        return 16383; // Valore centrale di default
    }

    bool readAll() {
        bool activityDetected = false;
        
        // Puliamo i bit prima di scriverci sopra (dal 28 al 31)
        currentEncButtons = 0;

        // --- ENCODER 1 (Standard) ---
        uint8_t res1 = encoder1.process();
        if (res1 == DIR_CW) { enc1_CW_Time = millis(); activityDetected = true; }
        if (res1 == DIR_CCW) { enc1_CCW_Time = millis(); activityDetected = true; }
        
        // Impulse Extender usando la costante ENC_HOLD_MS dal Config.h
        if (millis() - enc1_CW_Time < ENC_HOLD_MS) bitSet(currentEncButtons, 30);
        if (millis() - enc1_CCW_Time < ENC_HOLD_MS) bitSet(currentEncButtons, 31);
        
        // --- ENCODER 2 (Standard) ---
        uint8_t res2 = encoder2.process();
        if (res2 == DIR_CW) { enc2_CW_Time = millis(); activityDetected = true; }
        if (res2 == DIR_CCW) { enc2_CCW_Time = millis(); activityDetected = true; }
        
        if (millis() - enc2_CW_Time < ENC_HOLD_MS) bitSet(currentEncButtons, 28);
        if (millis() - enc2_CCW_Time < ENC_HOLD_MS) bitSet(currentEncButtons, 29);

        // --- ENCODER 3 e 4 (Potenziometri) ---
        uint8_t res3 = encoder3_pot.process();
        uint8_t res4 = encoder4_pot.process();
        
        if (res3 != 0 || res4 != 0) {
            activityDetected = true;
        }

        // Filtro EMA: 70% vecchio valore, 30% nuovo scatto.
        smoothAxis1 = (smoothAxis1 * 0.7) + (encoder3_pot.count * 0.3);
        smoothAxis2 = (smoothAxis2 * 0.7) + (encoder4_pot.count * 0.3);

        if (activityDetected) {
            PowerManager::resetActivity(); // Sveglia lo schermo
        }

        return activityDetected;
    }
}