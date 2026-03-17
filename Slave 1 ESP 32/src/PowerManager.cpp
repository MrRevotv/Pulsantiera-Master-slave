#include "PowerManager.h"

namespace PowerManager {
    int batteryPct = 100;
    unsigned long lastBatCheck = 0;
    unsigned long lastActivityTime = 0;
    bool screenActive = true;
    
    // Variabili per il tasto di accensione
    unsigned long powerBtnPressTime = 0;
    bool isPowerBtnHeld = false;
    bool turningOff = false;
    int countdown = 10;

    void init() {
        pinMode(POWER_BTN_PIN, INPUT_PULLUP);
        analogReadResolution(12);
        lastActivityTime = millis();
        update(); // Prima lettura immediata
    }

    void resetActivity() {
        lastActivityTime = millis();
        if (!screenActive) {
            turnOnScreen();
        }
    }

    int getBatteryPercentage() {
        return batteryPct;
    }

    bool isScreenOn() {
        return screenActive;
    }

    void turnOnScreen() {
        screenActive = true;
    }

    bool isTurningOff() {
        return turningOff;
    }

    int getTurnOffCountdown() {
        return countdown;
    }

    void executeDeepSleep() {
        esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BTN_PIN, 0); 
        esp_deep_sleep_start();
    }

    void update() {
        // 1. Lettura Batteria (ogni 5 secondi)
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
                countdown = 10 - (((millis() - powerBtnPressTime) - 1000) / 1000);
                if (countdown <= 0) {
                    executeDeepSleep();
                }
            }
        } else {
            isPowerBtnHeld = false;
            turningOff = false;
        }
    }
}