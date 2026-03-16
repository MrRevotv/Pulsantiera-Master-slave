#include "DisplayManager.h"

#define CORP_NAME "REV.TECH" 

Adafruit_SH1106G display(128, 64, &Wire, -1);

void initDisplay() {
    Wire.begin(OLED_SDA, OLED_SCL);
    delay(250);
    if (!display.begin(i2c_Address, true)) {
        Serial.println("Errore OLED");
    }
    display.clearDisplay();
}

void drawBootSequence() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    
    // Rettangolo allargato e centrato per farci stare bene "REV.TECH"
    display.drawRect(32, 10, 64, 15, SH110X_WHITE);
    display.setCursor(38, 14);
    display.print(CORP_NAME);
    display.display();
    delay(800);

    String bootText[] = {
        "VISOR OS v3.23",
        "SYNCING UPLINK..."
    };

    for(int i=0; i<2; i++) {
        display.setCursor(0, 35 + (i * 10));
        display.print("> ");
        display.print(bootText[i]);
        display.display();
        delay(600); 
    }
    delay(1000);
}

void showWelcomeMessage() {
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
    delay(3000); 
}

void updateHUD(int batteryPct, bool isConnected, String currentCommand) {
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.print(CORP_NAME);
    display.print("/SY_1");
    
    display.setCursor(82, 0);
    if (isConnected) {
        display.print("LINK OK");
    } else {
        display.print("LINK KO");
    }
    display.drawLine(0, 9, 128, 9, SH110X_WHITE);

    display.setCursor(0, 14);
    display.print("DATA STREAM");

    display.drawRect(0, 24, 128, 15, SH110X_WHITE);

    if (currentCommand == "") {
        display.setCursor(20, 28);
        display.print("ATTESA INPUT");
        int dots = (millis() / 500) % 4; 
        for(int i=0; i<dots; i++) display.print(".");
    } else {
        display.setCursor(5, 28);
        display.print("> ");
        display.print(currentCommand);
    }

    // --- SEZIONE INFERIORE (Batteria Ingrandita) ---
    display.drawLine(0, 44, 128, 44, SH110X_WHITE);

    if (batteryPct < 15 && (millis() % 500 < 250)) {
        display.setCursor(0, 50);
        display.print("!WARN!");
    } else {
        display.setCursor(0, 50);
        display.print("PWR:");
    }
    
    // Disegno la batteria stile "Cella Energetica" ingrandita
    display.drawRect(36, 48, 60, 12, SH110X_WHITE); 
    display.fillRect(96, 51, 3, 6, SH110X_WHITE);   
    
    int batBar = map(batteryPct, 0, 100, 0, 56);
    display.fillRect(38, 50, batBar, 8, SH110X_WHITE);
    
    display.setCursor(104, 50);
    display.print(batteryPct);
    display.print("%");

    display.display();
}

void showPowerOffCountdown(int secondsLeft) {
    display.clearDisplay();
    display.setTextSize(1);
    
    display.drawRect(0, 0, 128, 64, SH110X_WHITE);
    display.drawRect(2, 2, 124, 60, SH110X_WHITE);

    display.setCursor(15, 15);
    display.print("FORCING SLEEP IN:");
    
    display.setTextSize(3);
    if (secondsLeft >= 10) {
        display.setCursor(45, 30);
    } else {
        display.setCursor(55, 30);
    }
    display.print(secondsLeft);
    
    display.display();
}

void turnOffScreen() {
    display.clearDisplay();
    display.display();
}

void showReleaseMsg() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    
    // Un semplice box di avviso
    display.drawRect(0, 0, 128, 64, SH110X_WHITE);
    display.drawRect(2, 2, 124, 60, SH110X_WHITE);
    
    display.setCursor(15, 20);
    display.println("SPEGNIMENTO...");
    
    display.setCursor(15, 35);
    display.println("RILASCIA IL TASTO");
    
    display.display();
}