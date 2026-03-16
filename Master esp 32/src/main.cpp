#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP32Encoder.h>
#include "SimGamepad.h"

USBHID hid;
SimGamepad Gamepad;

// --- CONFIGURAZIONE SCHERMO ---
#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PINOUT HARDWARE MASTER ---
// Encoder 1 (Mappati su PC: Rotazione = Tasti 1-2, Pressione = Tasto 3)
#define ENC1_CLK 4
#define ENC1_DT 5
#define ENC1_SW 6

// Encoder 2 (Mappati su PC: Rotazione = Tasti 4-5, Pressione = Tasto 6)
#define ENC2_CLK 7
#define ENC2_DT 10
#define ENC2_SW 11

// 16 Pulsanti Fisici (Mappati su PC: Tasti 7 - 22)
const int NUM_BUTTONS = 16;
const int buttonPins[NUM_BUTTONS] = {12, 13, 14, 15, 16, 17, 18, 21, 38, 39, 40, 41, 42, 47, 48, 45};
bool buttonStates[NUM_BUTTONS] = {false};

ESP32Encoder encoder1;
ESP32Encoder encoder2;
long lastEnc1 = 0;
long lastEnc2 = 0;

// --- VARIABILI DI STATO E UI ---
bool isDebugMode = false;
bool debugComboFired = false;
String lastActionText = "In attesa...";
unsigned long lastActionTime = 0;

// Variabili per mantenere i pulsanti degli encoder attivi per 50ms (per farli leggere a Windows)
unsigned long encoderButtonReleaseTime[129] = {0};

// Tracking degli Slave
bool slave1Connected = false;
unsigned long lastSlave1Msg = 0;

typedef struct struct_message {
    uint8_t slaveID;
    uint32_t buttons;
    int16_t axis1;
    int16_t axis2;
} struct_message;
struct_message incomingData;

// --- CALLBACK WIRELESS ---
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataPtr, int len) {
    memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
    
    if (incomingData.slaveID == 1) {
        slave1Connected = true;
        lastSlave1Msg = millis();
        for (int i = 0; i < 32; i++) {
            bool isPressed = (incomingData.buttons & (1UL << i)) != 0;
            Gamepad.setButton(33 + i, isPressed);
        }
        Gamepad.setAxis(2, incomingData.axis1);
        Gamepad.setAxis(3, incomingData.axis2);
    }
}

// --- FUNZIONE FEEDBACK VISIVO ---
void triggerAction(String text) {
    lastActionText = text;
    lastActionTime = millis();
}

// --- FUNZIONI DI DISEGNO SCHERMO ---
void drawBootScreen() {
    display.clearDisplay();
    
    // 1. Schermata MrRevo
    display.setTextColor(SH110X_WHITE); // Forziamo il colore bianco
    display.drawRect(0, 0, 128, 64, SH110X_WHITE); // Cornice esterna
    
    display.setTextSize(2);
    // (128 pixel tot - 72 pixel di scritta) / 2 = Partiamo da X: 28 per centrare
    display.setCursor(28, 15); 
    display.println("MrRevo");
    
    display.setTextSize(1);
    display.setCursor(13, 40); // Centrato perfettamente
    display.println("Inizializzazione...");
    
    display.display();
    delay(2000);

    // 2. Schermata BENTORNATO
    display.clearDisplay();
    // Allarghiamo il rettangolo di sfondo e lo centriamo
    display.fillRoundRect(2, 15, 124, 34, 5, SH110X_WHITE);
    display.setTextColor(SH110X_BLACK);
    
    display.setTextSize(2);
    // Spostiamo il cursore a sinistra (X=4) così la O rientra nei 128 pixel!
    display.setCursor(4, 25); 
    display.println("BENTORNATO");
    
    display.display();
    delay(2000);
    
    // Ripristiniamo il colore normale per il resto del programma
    display.setTextColor(SH110X_WHITE); 
}

void drawNormalScreen() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 0);
    display.println("MASTERBOX");
    display.drawLine(0, 18, 128, 18, SH110X_WHITE);

    display.setTextSize(1);
    // Se è passata un'azione da meno di 3 secondi, mostrala
    if (millis() - lastActionTime < 3000) {
        display.setCursor(0, 30);
        display.println("Ultima azione:");
        display.setTextSize(2);
        display.setCursor(0, 45);
        display.println(lastActionText);
    } else {
        // Schermata di attesa (Idle)
        display.setCursor(0, 30);
        display.println("Status: Pronto");
        display.setCursor(0, 45);
        if(slave1Connected) display.println("Sensori: Slave 1 OK");
        else display.println("Sensori: Nessun Slave");
    }
    display.display();
}

void drawDebugScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.fillRect(0, 0, 128, 10, SH110X_WHITE);
    display.setTextColor(SH110X_BLACK);
    display.setCursor(2, 1);
    display.println("--- DEBUG MODE ---");
    
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 15);
    display.print("Master Tasti: ");
    
    // Mostra quanti tasti fisici sono premuti
    int pressed = 0;
    for(int i=0; i<NUM_BUTTONS; i++) if(buttonStates[i]) pressed++;
    display.println(pressed);

    display.setCursor(0, 25);
    display.print("E1: "); display.print(lastEnc1);
    display.print(" E2: "); display.println(lastEnc2);

    display.drawLine(0, 35, 128, 35, SH110X_WHITE);
    display.setCursor(0, 40);
    display.print("Slave 1: ");
    display.println(slave1Connected ? "CONNESSO" : "DISCONNESSO");
    
    if(slave1Connected) {
        display.setCursor(0, 50);
        display.print("SL1 Assi: ");
        display.print(incomingData.axis1);
    }
    display.display();
}

// --- SETUP ---
void setup() {
    USB.productName("MasterBox simulatori"); 
    USB.manufacturerName("Custom Sim Gear"); 
    USB.PID(0x1234); 
    Gamepad.begin();
    USB.begin();

    // Setup OLED
    Wire.begin(8, 9); 
    delay(250);
    display.begin(i2c_Address, true);
    
    drawBootScreen(); // Animazione figa

    // Setup Encoders
    ESP32Encoder::useInternalWeakPullResistors = UP;
    encoder1.attachHalfQuad(ENC1_DT, ENC1_CLK);
    encoder2.attachHalfQuad(ENC2_DT, ENC2_CLK);
    
    pinMode(ENC1_SW, INPUT_PULLUP);
    pinMode(ENC2_SW, INPUT_PULLUP);

    // Setup Pulsanti Fisici
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }

    WiFi.mode(WIFI_STA);
    if (esp_now_init() == ESP_OK) {
        esp_now_register_recv_cb(OnDataRecv);
    }
}

// --- LOOP PRINCIPALE ---
void loop() {
    // 1. Lettura Encoders
    long newEnc1 = encoder1.getCount() / 2; // Diviso 2 perché usiamo HalfQuad
    if (newEnc1 > lastEnc1) { Gamepad.setButton(1, true); encoderButtonReleaseTime[1] = millis() + 50; triggerAction("Enc 1 DX"); }
    if (newEnc1 < lastEnc1) { Gamepad.setButton(2, true); encoderButtonReleaseTime[2] = millis() + 50; triggerAction("Enc 1 SX"); }
    lastEnc1 = newEnc1;

    long newEnc2 = encoder2.getCount() / 2;
    if (newEnc2 > lastEnc2) { Gamepad.setButton(4, true); encoderButtonReleaseTime[4] = millis() + 50; triggerAction("Enc 2 DX"); }
    if (newEnc2 < lastEnc2) { Gamepad.setButton(5, true); encoderButtonReleaseTime[5] = millis() + 50; triggerAction("Enc 2 SX"); }
    lastEnc2 = newEnc2;

    // Spegnimento automatico finti bottoni encoder dopo 50ms
    for(int i=1; i<=128; i++) {
        if(encoderButtonReleaseTime[i] > 0 && millis() > encoderButtonReleaseTime[i]) {
            Gamepad.setButton(i, false);
            encoderButtonReleaseTime[i] = 0;
        }
    }

    // 2. Lettura Pulsanti Encoder
    bool enc1SwPressed = !digitalRead(ENC1_SW);
    bool enc2SwPressed = !digitalRead(ENC2_SW);
    Gamepad.setButton(3, enc1SwPressed);
    Gamepad.setButton(6, enc2SwPressed);
    
    // Feedback testo per pressione singola (non spammarlo se tenuto premuto)
    static bool prevE1 = false, prevE2 = false;
    if(enc1SwPressed && !prevE1) triggerAction("Click E1"); prevE1 = enc1SwPressed;
    if(enc2SwPressed && !prevE2) triggerAction("Click E2"); prevE2 = enc2SwPressed;

    // 3. Lettura 16 Pulsanti Fisici Master
    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool currentState = !digitalRead(buttonPins[i]);
        if (currentState != buttonStates[i]) {
            buttonStates[i] = currentState;
            Gamepad.setButton(7 + i, currentState);
            if (currentState) triggerAction("Tasto " + String(i + 1));
        }
    }

    // 4. CHECK COMBO SEGRETA (Encoder 1 SW + Tasto Fisico 1)
    if (enc1SwPressed && buttonStates[0]) {
        if (!debugComboFired) {
            isDebugMode = !isDebugMode; // Inverte lo stato
            debugComboFired = true;
            triggerAction(isDebugMode ? "DEBUG ON" : "DEBUG OFF");
        }
    } else {
        debugComboFired = false;
    }

    // 5. Controllo Timeout Slave (Se non parla da 2 secondi, lo dichiariamo disconnesso)
    if (slave1Connected && millis() - lastSlave1Msg > 2000) {
        slave1Connected = false;
    }

    // 6. Aggiornamento Schermo
    if (isDebugMode) {
        drawDebugScreen();
    } else {
        drawNormalScreen();
    }

    // Invia i dati al PC
    Gamepad.send();
    delay(10); 
}