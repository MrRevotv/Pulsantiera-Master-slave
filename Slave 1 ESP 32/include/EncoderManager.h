#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "CustomEncoder.h"

namespace EncoderManager {
    void init();
    bool readAll(); // Ritorna 'true' se un encoder è stato mosso
    uint32_t getButtonsState();
    int16_t getAxis(int axisNumber);
}

#endif