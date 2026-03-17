#ifndef COMMS_MANAGER_H
#define COMMS_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "Config.h"

namespace CommsManager {
    void init(const uint8_t* masterMac);
    void sendIfReady(struct_message& dataToSend);
    bool isConnected();
}

#endif