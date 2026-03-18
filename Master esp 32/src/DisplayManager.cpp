#include "DisplayManager.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "Config.h"

#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define CORP_NAME "REV.TECH"

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initDisplay() {
    Wire.begin(8, 9); // Pin I2C del Master
    delay(250);
    if (!display.begin(i2c_Address, true)) {
        Serial.println("Errore OLED");
    }
}

void drawBootScreen() {
    // 1. Schermata Iniziale REV.TECH
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    
    display.drawRect(32, 10, 64, 15, SH110X_WHITE);
    display.setCursor(38, 14);
    display.print(CORP_NAME);
    display.display();
    delay(800);

    // 2. Sequenza di Boot
    String bootText[] = {
        "MASTER OS v1.00",
        "INIT USB LINK..."
    };

    for(int i=0; i<2; i++) {
        display.setCursor(0, 35 + (i * 10));
        display.print("> ");
        display.print(bootText[i]);
        display.display();
        delay(600); 
    }
    delay(1000);

    // 3. Schermata di Benvenuto
    display.clearDisplay();
    display.drawLine(0, 0, 128, 0, SH110X_WHITE);
    display.drawLine(0, 63, 128, 63, SH110X_WHITE);
    display.drawLine(0, 0, 0, 15, SH110X_WHITE);
    display.drawLine(127, 48, 127, 63, SH110X_WHITE);

    display.setTextSize(1);
    display.setCursor(10, 20);
    display.println("LOGIN UTENTE");
    display.setCursor(10, 35);
    display.println("BENTORNATO, MRREVO");
    display.display();
    delay(2000); 
}

void drawHUD() {
    display.clearDisplay();
    display.setTextSize(1);

    // --- HEADER FISSO ---
    display.setCursor(0, 0);
    display.print(CORP_NAME);
    display.print("//MASTER 3.23");
    display.drawLine(0, 9, 128, 9, SH110X_WHITE);

    // --- LOGICA SCHERMO PRINCIPALE ---
    // Se è stata fatta un'azione negli ultimi 5 secondi, mostra l'input
    if (millis() - sysState.lastActionTime <= 5000 && sysState.lastActionText != "") {
        display.setCursor(0, 14);
        display.print("DATA STREAM");

        display.drawRect(0, 24, 128, 15, SH110X_WHITE);
        display.setCursor(5, 28);
        display.print("> ");
        display.print(sysState.lastActionText);
    } 
    // Altrimenti entra in ciclo di IDLE (Stand-by)
    else {
        // Pulisce il testo vecchio per sicurezza
        sysState.lastActionText = ""; 
        
        bool anySlave = sysState.slave1Connected; // Aggiungi qui || sysState.slave2Connected || sysState.slave3Connected quando pronti
        int cycle = (millis() / 3000) % 2; // Cicla tra 0 e 1 ogni 3 secondi

        if (cycle == 0 || !anySlave) {
            // FASE 1: Logo Operativo Figo
            display.setTextSize(2);
            display.setCursor(10, 22);
            display.print("MASTERBOX");
            
            display.setTextSize(1);
            display.setCursor(30, 42);
            display.print("OPERATIVA");
        } else {
            // FASE 2: Mostra dinamicamente quali Slave sono attivi
            display.setCursor(0, 14);
            display.print("UPLINK ATTIVI:");
            
            int yPos = 26;
            if (sysState.slave1Connected) { display.setCursor(5, yPos); display.print("> SLAVE 1 [OK]"); yPos += 10; }
            // Quando avrai gli altri slave, de-commenta queste righe:
            // if (sysState.slave2Connected) { display.setCursor(5, yPos); display.print("> SLAVE 2 [OK]"); yPos += 10; }
            // if (sysState.slave3Connected) { display.setCursor(5, yPos); display.print("> SLAVE 3 [OK]"); yPos += 10; }
        }
    }

    // --- FOOTER ESTETICO ---
    display.drawLine(0, 54, 128, 54, SH110X_WHITE);
    display.setCursor(0, 56);
    display.print("SYS_RDY");
    
    // Piccolo indicatore di "caricamento" animato in basso a destra per far capire che il sistema è vivo
    int loadingDots = (millis() / 250) % 4;
    display.setCursor(100, 56);
    if(loadingDots == 0) display.print("|");
    if(loadingDots == 1) display.print("/");
    if(loadingDots == 2) display.print("-");
    if(loadingDots == 3) display.print("\\");

    display.display();
}

void drawDebugScreen(unsigned long debugActiveTime) {
    display.clearDisplay();
    display.setTextSize(1);
    
    // Header Invertito (Sfondo bianco, testo nero)
    display.fillRect(0, 0, 128, 10, SH110X_WHITE);
    display.setTextColor(SH110X_BLACK);
    display.setCursor(2, 1);
    display.println("--- DEBUG MODE ---");
    display.setTextColor(SH110X_WHITE);

    // --- LOGICA TIMER 15 SECONDI ---
    if (debugActiveTime < 15000) {
        // PRIMI 15 SECONDI: Mostra le connessioni
        display.setCursor(0, 15);
        display.println("DIAGNOSTICA UPLINK:");
        
        display.setCursor(0, 28);
        display.print("SL1: ");
        display.println(sysState.slave1Connected ? "[ ON-LINE ]" : "[ OFF-LINE ]");
        
        // Puoi aggiungere SL2 e SL3 qui sotto
        
        // Mostra il conto alla rovescia in basso
        display.setCursor(0, 55);
        display.print("Switch dati in: ");
        display.print(15 - (debugActiveTime / 1000));
        display.print("s");
    } else {
        // DOPO I 15 SECONDI: Mostra gli input
        display.setCursor(0, 15);
        display.print("MST Tasti: ");
        display.println(sysState.pressedButtonsCount);

        display.setCursor(0, 25);
        display.print("E1: "); display.print(sysState.enc1Count);
        display.print(" E2: "); display.println(sysState.enc2Count);

        display.drawLine(0, 35, 128, 35, SH110X_WHITE);
        
        if(sysState.slave1Connected) {
            display.setCursor(0, 40);
            display.println("SL1 INPUTS:");
            display.setCursor(0, 50);
            display.print("A1: "); display.print(sysState.slave1Axis1);
        } else {
            display.setCursor(0, 45);
            display.println("SL1: NO DATA");
        }
    }
    
    display.display();
}

void updateDisplay() {
    // Variabili statiche per ricordare lo stato precedente e il timer del debug
    static bool lastDebugState = false;
    static unsigned long debugStartTime = 0;

    // Rileva IL MOMENTO in cui entriamo in debug mode per far partire il timer da 0
    if (sysState.isDebugMode && !lastDebugState) {
        debugStartTime = millis();
    }
    lastDebugState = sysState.isDebugMode;

    if (sysState.isDebugMode) {
        drawDebugScreen(millis() - debugStartTime); // Passa il tempo trascorso
    } else {
        drawHUD();
    }
}