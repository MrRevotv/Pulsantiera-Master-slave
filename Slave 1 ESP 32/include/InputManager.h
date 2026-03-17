#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>
#include <Keypad.h>
#include "Config.h"

namespace InputManager {
    void init();
    bool readAll(); // Ritorna 'true' se un tasto qualsiasi è stato premuto/rilasciato
    uint32_t getButtonsState();
    String getLastActionText();
}

#endif