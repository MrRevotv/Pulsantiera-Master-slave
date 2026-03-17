#ifndef FEEDBACK_MANAGER_H
#define FEEDBACK_MANAGER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

namespace FeedbackManager {
    void init();
    
    // Gestione Schermo OLED
    void showBootScreen();
    void showWelcomeMessage();
    void showReleaseMsg();
    void showPowerOffCountdown(int secondsLeft);
    void turnOffScreen();
    void updateHUD(int batteryPct, bool isConnected, String lastAction);
}

#endif