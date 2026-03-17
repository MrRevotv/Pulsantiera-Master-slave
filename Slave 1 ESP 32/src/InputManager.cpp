#include "InputManager.h"
#include "PowerManager.h" // Per poter resettare il timer di inattività

namespace InputManager {
    
    char keys[MATRIX_ROWS][MATRIX_COLS] = {
        {0, 1, 2, 3}, 
        {4, 5, 6, 7}, 
        {8, 9, 10, 11}, 
        {12, 13, 14, 15}
    };
    
    // Nota: Il cast a (byte*) serve per compatibilità con la libreria Keypad
    Keypad matrixKeypad = Keypad(makeKeymap(keys), (byte*)ROW_PINS, (byte*)COL_PINS, MATRIX_ROWS, MATRIX_COLS);
    
    uint32_t currentButtonsState = 0;
    uint32_t previousButtonsState = 0;
    String lastActionText = "";

    void init() {
        // I pin della matrice sono gestiti in automatico dalla libreria Keypad
        
        // Pin dei click degli encoder
        pinMode(ENC1_SW, INPUT); // Ricorda: questo ha bisogno della resistenza HW
        pinMode(ENC2_SW, INPUT); // Ricorda: questo ha bisogno della resistenza HW
        pinMode(ENC3_SW, INPUT); // Ricorda: questo ha bisogno della resistenza HW
        pinMode(ENC4_SW, INPUT_PULLUP); // Questo ha il pullup interno sul pin 0
    }

    uint32_t getButtonsState() {
        return currentButtonsState;
    }

    String getLastActionText() {
        return lastActionText;
    }

    bool readAll() {
        bool stateChanged = false;
        
        // 1. Leggi la Matrice
        if (matrixKeypad.getKeys()) {
            for (int i=0; i<LIST_MAX; i++) {
                if (matrixKeypad.key[i].stateChanged) {
                    if (matrixKeypad.key[i].kstate == PRESSED || matrixKeypad.key[i].kstate == HOLD) {
                        bitSet(currentButtonsState, matrixKeypad.key[i].kchar);
                    }
                    if (matrixKeypad.key[i].kstate == RELEASED || matrixKeypad.key[i].kstate == IDLE) {
                        bitClear(currentButtonsState, matrixKeypad.key[i].kchar);
                    }
                    stateChanged = true;
                }
            }
        }

        // 2. Leggi i Click degli Encoder (Bit 16, 17, 18, 19)
        // Usiamo !digitalRead perché i tasti chiudono a massa (LOW)
       // if (!digitalRead(ENC1_SW)) bitSet(currentButtonsState, 16); else bitClear(currentButtonsState, 16);
       // if (!digitalRead(ENC2_SW)) bitSet(currentButtonsState, 17); else bitClear(currentButtonsState, 17);Me li sono disabilitati temporaneamente in attessa che arrivi il materiale.
        if (!digitalRead(ENC3_SW)) bitSet(currentButtonsState, 18); else bitClear(currentButtonsState, 18);
        if (!digitalRead(ENC4_SW)) bitSet(currentButtonsState, 19); else bitClear(currentButtonsState, 19);

        // 3. Verifica se c'è stato un cambiamento rispetto all'ultimo ciclo
        if (currentButtonsState != previousButtonsState) {
            stateChanged = true;
            PowerManager::resetActivity(); // Sveglia lo schermo e resetta il timer
            
            // Genera il testo per l'HUD (solo per la pressione, non per il rilascio)
            for (int i = 0; i < 20; i++) {
                if (bitRead(currentButtonsState, i) && !bitRead(previousButtonsState, i)) {
                    if (i == 16) lastActionText = "ENC 1 (CLICK)";
                    else if (i == 17) lastActionText = "ENC 2 (CLICK)";
                    else if (i == 18) lastActionText = "ENC 3 (CLICK)";
                    else if (i == 19) lastActionText = "ENC 4 (CLICK)";
                    else lastActionText = "TASTO " + String(i + 1); 
                }
            }
            previousButtonsState = currentButtonsState;
        }

        return stateChanged;
    }
} // <-- Parentesi extra rimossa!