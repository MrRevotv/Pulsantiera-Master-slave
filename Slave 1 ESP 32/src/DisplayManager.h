#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define OLED_SDA 26
#define OLED_SCL 27
#define i2c_Address 0x3c

void initDisplay();
void drawBootSequence();
void showWelcomeMessage();
void updateHUD(int batteryPct, bool isConnected, String currentCommand);
void showPowerOffCountdown(int secondsLeft);
void turnOffScreen(); 

void showReleaseMsg();

#endif