// File: include/SimGamepad.h
#pragma once

#include <Arduino.h>
#include "USB.h"
#include "USBHID.h"

// Dichiariamo che l'oggetto hid esisterà nel file principale
extern USBHID hid;

// 1. DESCRITTORE USB (128 Pulsanti, 8 Assi)
static const uint8_t hid_report_descriptor[] = {
    0x05, 0x01, 0x09, 0x04, 0xA1, 0x01, 0x85, 0x01, 
    0x05, 0x09, 0x19, 0x01, 0x29, 0x80, 0x15, 0x00, 
    0x25, 0x01, 0x95, 0x80, 0x75, 0x01, 0x81, 0x02, 
    0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 
    0x09, 0x33, 0x09, 0x34, 0x09, 0x35, 0x09, 0x36, 
    0x09, 0x37, 0x16, 0x00, 0x80, 0x26, 0xFF, 0x7F, 
    0x75, 0x10, 0x95, 0x08, 0x81, 0x02, 0xC0
};

// 2. CLASSE GAMEPAD
class SimGamepad : public USBHIDDevice {
private:
    uint8_t buttons[16]; 
    int16_t axes[8];     
public:
    SimGamepad() {
        memset(buttons, 0, sizeof(buttons));
        memset(axes, 0, sizeof(axes));
        hid.addDevice(this, sizeof(hid_report_descriptor));
    }
    void begin() { hid.begin(); }
    uint16_t _onGetDescriptor(uint8_t* buffer) {
        memcpy(buffer, hid_report_descriptor, sizeof(hid_report_descriptor));
        return sizeof(hid_report_descriptor);
    }
    void setButton(uint8_t button, bool state) {
        if(button < 1 || button > 128) return;
        button--; 
        if(state) buttons[button / 8] |= (1 << (button % 8));
        else      buttons[button / 8] &= ~(1 << (button % 8));
    }
    void setAxis(uint8_t axisIndex, int16_t value) {
        if(axisIndex > 7) return;
        axes[axisIndex] = value;
    }
    void send() {
        uint8_t report[32];
        memcpy(&report[0], buttons, 16);
        memcpy(&report[16], axes, 16);
        hid.SendReport(1, report, sizeof(report));
    }
};