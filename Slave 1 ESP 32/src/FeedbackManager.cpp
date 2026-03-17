#include "FeedbackManager.h"

// Configurazione per il tuo schermo SH1106 da 1.3 pollici
// Inserisci i numeri veri al posto di SCL_PIN e SDA_PIN (es. 27 e 26)
U8G2_SH1106_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=SCL*/ 27, /* data=SDA*/ 26, /* reset=*/ U8X8_PIN_NONE);

namespace FeedbackManager {
    
    String currentAction = "";
    unsigned long actionTimer = 0;

    void init() {
        u8g2.begin();
    }

    void showBootScreen() {
        // Semplice scritta di avvio testuale (sostituisce il tuo vecchio drawBootSequence)
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB14_tr);
            u8g2.drawStr(10, 40, "AVVIO...");
        } while (u8g2.nextPage());
        delay(2000); 
    }

    void showWelcomeMessage() {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB14_tr);
            u8g2.drawStr(15, 40, "PANNELLO ON");
        } while (u8g2.nextPage());
        delay(1000);
    }

    void showReleaseMsg() {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB10_tr);
            u8g2.drawStr(10, 40, "RILASCIA TASTO");
        } while (u8g2.nextPage());
    }

    void showPowerOffCountdown(int secondsLeft) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB14_tr);
            u8g2.drawStr(10, 30, "SPEGNIMENTO");
            u8g2.setCursor(60, 60);
            u8g2.print(secondsLeft);
        } while (u8g2.nextPage());
    }

    void turnOffScreen() {
        u8g2.clearDisplay();
    }

    void updateHUD(int batteryPct, bool isConnected, String newAction) {
        // Aggiorna il testo dell'azione solo se è nuovo e fallo durare 5 secondi
        if (newAction != "") {
            currentAction = newAction;
            actionTimer = millis();
        }
        if (millis() - actionTimer > 5000) {
            currentAction = ""; // Pulisci dopo 5 secondi
        }

        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB08_tr);
            
            // 1. Disegna la Batteria (in alto a sinistra)
            u8g2.setCursor(0, 10);
            u8g2.print("BAT: ");
            u8g2.print(batteryPct);
            u8g2.print("%");

            // 2. Disegna lo stato di Connessione (in alto a destra)
            u8g2.setCursor(90, 10);
            if (isConnected) {
                u8g2.print("WIFI");
            } else {
                u8g2.print("----");
            }

            // 3. Disegna l'ultima azione (al centro in grande)
            u8g2.setFont(u8g2_font_ncenB10_tr);
            int textWidth = u8g2.getStrWidth(currentAction.c_str());
            u8g2.setCursor((128 - textWidth) / 2, 45); // Centrato
            u8g2.print(currentAction);

        } while (u8g2.nextPage());
    }
}