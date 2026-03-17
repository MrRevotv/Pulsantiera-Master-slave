#include "PowerManager.h"
#include "FeedbackManager.h" // Aggiunto per comunicare con lo schermo prima della nanna

namespace PowerManager {
    int batteryPct = 100;
    unsigned long lastBatCheck = 0;
    unsigned long lastActivityTime = 0;
    bool screenActive = true;
    
    unsigned long powerBtnPressTime = 0;
    bool isPowerBtnHeld = false;
    bool turningOff = false;
    int countdown = 3; // Modificato da 10 a 3 secondi

    void init() {
        pinMode(POWER_BTN_PIN, INPUT_PULLUP);
        analogReadResolution(12);
        lastActivityTime = millis();
        update(); 
    }

    void resetActivity() {
        lastActivityTime = millis();
        if (!screenActive) {
            turnOnScreen();
        }
    }

    int getBatteryPercentage() { return batteryPct; }
    bool isScreenOn() { return screenActive; }
    void turnOnScreen() { screenActive = true; }
    bool isTurningOff() { return turningOff; }
    int getTurnOffCountdown() { return countdown; }

    void executeDeepSleep() {
        // 1. Spegne i pixel dello schermo
        FeedbackManager::turnOffScreen();
        delay(100); // Diamo tempo alla comunicazione I2C di arrivare

        // 2. Aspetta che l'utente rilasci il tasto prima di dormire
        while(!digitalRead(POWER_BTN_PIN)) {
            delay(50);
        }
        delay(200); // Piccolo debounce di sicurezza
        
        // 3. Buonanotte
        esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BTN_PIN, 0); 
        esp_deep_sleep_start();
    }

    void update() {
        // 1. Lettura Batteria
        if (millis() - lastBatCheck > 5000) {
            int raw = analogRead(BATTERY_PIN);
            float voltage = (raw / 4095.0) * 2.0 * 3.3 * 1.1; 
            batteryPct = constrain(map(voltage * 100, 330, 420, 0, 100), 0, 100);
            lastBatCheck = millis();
        }

        // 2. Controllo Standby (Inattività)
        if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
            executeDeepSleep();
        }
        if (millis() - lastActivityTime > SCREEN_TIMEOUT && screenActive) { 
            screenActive = false; 
        }

        // 3. Controllo Tasto di Accensione / Spegnimento
        bool powerBtnState = !digitalRead(POWER_BTN_PIN); 
        
        if (powerBtnState) {
            if (!isPowerBtnHeld) { 
                isPowerBtnHeld = true; 
                powerBtnPressTime = millis(); 
            }
            if (!screenActive) { resetActivity(); }

            if (millis() - powerBtnPressTime > 1000) {
                turningOff = true;
                // Calcolo scalato per 3 secondi invece che 10
                countdown = 3 - (((millis() - powerBtnPressTime) - 1000) / 1000); 
            }
        } else {
            isPowerBtnHeld = false;
            turningOff = false;
        }
    }
}