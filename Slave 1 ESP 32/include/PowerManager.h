#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "Config.h"

namespace PowerManager {
    void init();
    void update();
    void resetActivity();
    int getBatteryPercentage();
    
    // Funzioni per gestire lo spegnimento
    bool isScreenOn();
    void turnOnScreen();
    bool isTurningOff();
    int getTurnOffCountdown();
    void executeDeepSleep();
}

#endif