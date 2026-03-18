#include <Arduino.h>
#include "Config.h"
#include "SimGamepad.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include "CommsManager.h"
#include "BuzzerManager.h" // Aggiunto

USBHID hid;
SimGamepad Gamepad;
SystemState sysState;         // Istanza globale dello stato
struct_message incomingData;  // Istanza globale dei dati in arrivo

void setup() {
    // 1. Setup USB
    USB.productName("MasterBox simulatori"); 
    USB.manufacturerName("Custom Sim Gear"); 
    USB.PID(0x1234); 
    Gamepad.begin();
    USB.begin();

    // 2. Setup Schermo
    initDisplay();
    drawBootScreen();

    // 3. Setup Hardware e Rete
    initInputs();
    initComms();
    initBuzzer(); // Inizializza il pin del buzzer
}

void loop() {
    readInputs();           // Legge tasti ed encoder (Master)
    checkCommsTimeout();    // Controlla se gli Slave sono vivi
    updateDisplay();        // Aggiorna la UI (Normale o Debug)
    handleBuzzer();         // Gestisce i bip senza bloccare il loop
    
    Gamepad.send();         // Invia i dati a Windows
    delay(10); 
}