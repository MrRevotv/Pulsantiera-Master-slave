#ifndef FEEDBACK_MANAGER_H
#define FEEDBACK_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

namespace FeedbackManager {
    void init();
    void showBootScreen();
    void showWelcomeMessage();
    void updateHUD(int batteryPct, bool isConnected, String newAction);
    void showPowerOffCountdown(int secondsLeft);
    void turnOffScreen();
    void showReleaseMsg();
}

#endif