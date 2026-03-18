#include "CommsManager.h"
#include <WiFi.h>
#include <esp_now.h>
#include "Config.h"
#include "SimGamepad.h"

extern SimGamepad Gamepad;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataPtr, int len) {
    memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
    
    if (incomingData.slaveID == 1) {
        if (!sysState.slave1Connected) {
            sysState.triggerConnectBeep = true; // Nuova connessione!
        }
        sysState.slave1Connected = true;
        sysState.lastSlave1Msg = millis();
        sysState.slave1Axis1 = incomingData.axis1;
        
        for (int i = 0; i < 32; i++) {
            bool isPressed = (incomingData.buttons & (1UL << i)) != 0;
            Gamepad.setButton(33 + i, isPressed);
        }
        Gamepad.setAxis(2, incomingData.axis1);
        Gamepad.setAxis(3, incomingData.axis2);
    }
    else if (incomingData.slaveID == 2) {
        if (!sysState.slave2Connected) {
            sysState.triggerConnectBeep = true;
        }
        sysState.slave2Connected = true;
        sysState.lastSlave2Msg = millis();
        sysState.slave2Axis1 = incomingData.axis1;
        // Gestisci input per Slave 2...
    }
    else if (incomingData.slaveID == 3) {
        if (!sysState.slave3Connected) {
            sysState.triggerConnectBeep = true;
        }
        sysState.slave3Connected = true;
        sysState.lastSlave3Msg = millis();
        sysState.slave3Axis1 = incomingData.axis1;
        // Gestisci input per Slave 3...
    }
}

void initComms() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() == ESP_OK) {
        esp_now_register_recv_cb(OnDataRecv);
    }
}

void checkCommsTimeout() {
    unsigned long currentTime = millis(); 
    
    // Controllo Timeout Slave 1
    if (sysState.slave1Connected && (currentTime - sysState.lastSlave1Msg > 2000)) {
        sysState.slave1Connected = false;
        sysState.triggerDisconnectBeep = true; // Disconnesso!
    }
    
    // Controllo Timeout Slave 2
    if (sysState.slave2Connected && (currentTime - sysState.lastSlave2Msg > 2000)) {
        sysState.slave2Connected = false;
        sysState.triggerDisconnectBeep = true; // Disconnesso!
    }
    
    // Controllo Timeout Slave 3
    if (sysState.slave3Connected && (currentTime - sysState.lastSlave3Msg > 2000)) {
        sysState.slave3Connected = false;
        sysState.triggerDisconnectBeep = true; // Disconnesso!
    }
}