#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Keypad.h>
#include "DisplayManager.h" 
#include "CustomEncoder.h" 

// --- MAC ADDRESS MASTER ---
uint8_t broadcastAddress[] = {0xDC, 0xB4, 0xD9, 0x38, 0x20, 0x54}; 

// --- PIN SPECIALI ---
#define BATTERY_PIN 35
#define POWER_BTN_PIN 15 
#define ENC1_SW 34
#define ENC2_SW 36
#define ENC3_SW 39
#define ENC4_SW 0

// --- TIMERS E COSTANTI ---
#define SCREEN_TIMEOUT 1200000UL 
#define SLEEP_TIMEOUT 3600000UL  
const int ENC_HOLD_MS = 60; // Millisecondi in cui il pulsante rotazione rimane "premuto" per Virpil

bool isScreenOn = true;

// --- SETUP MATRICE ---
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {0, 1, 2, 3}, 
  {4, 5, 6, 7}, 
  {8, 9, 10, 11}, 
  {12, 13, 14, 15}
};
byte rowPins[ROWS] = {32, 33, 25, 14}; 
byte colPins[COLS] = {13, 12, 4, 2}; 
Keypad matrixKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
uint32_t currentMatrixState = 0;

// --- STRUTTURA DATI ESP-NOW ---
typedef struct struct_message {
    uint8_t slaveID;
    uint32_t buttons;
    int16_t axis1;
    int16_t axis2;
} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;
bool masterConnected = false; 

// --- INIZIALIZZAZIONE ENCODER (ttable) ---
CustomEncoder encoder1(5, 18, false);       
CustomEncoder encoder2(19, 21, false);      
CustomEncoder encoder3_pot(22, 23, true);  
CustomEncoder encoder4_pot(1, 3, true);    

// --- VARIABILI DI STATO ---
int batteryPct = 100;
unsigned long lastBatCheck = 0;
unsigned long lastActivityTime = 0; 
String lastActionText = "";
unsigned long lastActionTime = 0;
uint32_t lastButtonsState = 0;

// Variabili per mantenere premuti gli encoder abbastanza a lungo per ESP-NOW
unsigned long enc1_CW_Time = 0;
unsigned long enc1_CCW_Time = 0;
unsigned long enc2_CW_Time = 0;
unsigned long enc2_CCW_Time = 0;

// --- FUNZIONI ---
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  masterConnected = (status == ESP_NOW_SEND_SUCCESS);
}

int getBatteryPercentage() {
    int raw = analogRead(BATTERY_PIN);
    float voltage = (raw / 4095.0) * 2.0 * 3.3 * 1.1; 
    return constrain(map(voltage * 100, 330, 420, 0, 100), 0, 100);
}

void setup() {
// 1. RIABILITA LA SERIALE PER IL DEBUG
    Serial.begin(115200);
    delay(1000); // Piccola pausa per far avviare il monitor seriale
    Serial.println("--- AVVIO SLAVE DEBUGER ---");    
    initDisplay();
    drawBootSequence();

    pinMode(POWER_BTN_PIN, INPUT_PULLUP);
    pinMode(ENC1_SW, INPUT);
    pinMode(ENC2_SW, INPUT);
    pinMode(ENC3_SW, INPUT);
    pinMode(ENC4_SW, INPUT_PULLUP); 

    encoder1.begin();
    encoder2.begin();
    encoder3_pot.begin();
    encoder4_pot.begin();

    analogReadResolution(12);

    WiFi.mode(WIFI_STA);
    if (esp_now_init() == ESP_OK) {
        esp_now_register_send_cb(OnDataSent);
        memcpy(peerInfo.peer_addr, broadcastAddress, 6);
        peerInfo.channel = 0;  
        peerInfo.encrypt = false;
        esp_now_add_peer(&peerInfo);
    }

    myData.slaveID = 1;
    lastActivityTime = millis(); 
}

void loop() {
    bool userDidSomething = false;

    // --- 0. GESTIONE ACCENSIONE ---
    bool powerBtnState = !digitalRead(POWER_BTN_PIN); 
    static unsigned long powerBtnPressTime = 0;
    static bool isPowerBtnHeld = false;

    if (powerBtnState) {
        if (!isPowerBtnHeld) { isPowerBtnHeld = true; powerBtnPressTime = millis(); }
        if (!isScreenOn) { isScreenOn = true; lastActivityTime = millis(); showWelcomeMessage(); }

        if (millis() - powerBtnPressTime > 1000) {
            int secondsLeft = 10 - (((millis() - powerBtnPressTime) - 1000) / 1000);
            if (secondsLeft <= 0) {
                showReleaseMsg(); 
                while (!digitalRead(POWER_BTN_PIN)) delay(50);
                delay(500);
                turnOffScreen(); 
                esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BTN_PIN, 0); 
                esp_deep_sleep_start();
            } else {
                showPowerOffCountdown(secondsLeft);
                return; 
            }
        }
    } else if (isPowerBtnHeld) {
        isPowerBtnHeld = false; lastActivityTime = millis(); 
    }

    // --- 1. MATRICE (Azzera i bit della matrice per leggerli da capo) ---
    if (matrixKeypad.getKeys()) {
        for (int i=0; i<LIST_MAX; i++) {
            if (matrixKeypad.key[i].stateChanged) {
                if (matrixKeypad.key[i].kstate == PRESSED || matrixKeypad.key[i].kstate == HOLD) bitSet(currentMatrixState, matrixKeypad.key[i].kchar);
                if (matrixKeypad.key[i].kstate == RELEASED || matrixKeypad.key[i].kstate == IDLE) bitClear(currentMatrixState, matrixKeypad.key[i].kchar);
            }
        }
    }
    // Copia i tasti della matrice puliti nel pacchetto (questo cancella i bit 16-31)
    myData.buttons = currentMatrixState; 

    // --- 2. CLICK ENCODER ---
    if (!digitalRead(ENC1_SW)) { bitSet(myData.buttons, 16); Serial.println("Premuto: SW1 (Pin 34)"); }
    if (!digitalRead(ENC2_SW)) { bitSet(myData.buttons, 17); Serial.println("Premuto: SW2 (Pin 36)"); }
    if (!digitalRead(ENC3_SW)) { bitSet(myData.buttons, 18); Serial.println("Premuto: SW3 (Pin 39)"); }
    if (!digitalRead(ENC4_SW)) bitSet(myData.buttons, 19);

// --- 3. LETTURA ROTAZIONE E "IMPULSE EXTENDER" ---
    
    // Encoder 1
    uint8_t res1 = encoder1.process();
    if (res1 == DIR_CW) {
        enc1_CW_Time = millis();
        Serial.println("Encoder 1: Scatto a DESTRA (Tasto 30)");
    }
    if (res1 == DIR_CCW) {
        enc1_CCW_Time = millis();
        Serial.println("Encoder 1: Scatto a SINISTRA (Tasto 31)");
    }
    
    if (millis() - enc1_CW_Time < ENC_HOLD_MS) bitSet(myData.buttons, 30);
    if (millis() - enc1_CCW_Time < ENC_HOLD_MS) bitSet(myData.buttons, 31);
    
    // Encoder 2
    uint8_t res2 = encoder2.process();
    if (res2 == DIR_CW) {
        enc2_CW_Time = millis();
        Serial.println("Encoder 2: Scatto a DESTRA (Tasto 28)");
    }
    if (res2 == DIR_CCW) {
        enc2_CCW_Time = millis();
        Serial.println("Encoder 2: Scatto a SINISTRA (Tasto 29)");
    }
    
    if (millis() - enc2_CW_Time < ENC_HOLD_MS) bitSet(myData.buttons, 28);
    if (millis() - enc2_CCW_Time < ENC_HOLD_MS) bitSet(myData.buttons, 29);


    // --- 4. ASSI POTENZIOMETRO E "SMOOTHING" ---
    uint8_t res3 = encoder3_pot.process();
    // encoder4_pot.process(); // Ometti il 4 finché usi il Monitor Seriale

    if (res3 == DIR_CW) {
        Serial.print("Potenziometro 3: SU -> Valore: ");
        Serial.println(encoder3_pot.count);
    }
    if (res3 == DIR_CCW) {
        Serial.print("Potenziometro 3: GIU -> Valore: ");
        Serial.println(encoder3_pot.count);
    }

    static float smoothAxis1 = 16383.0;
    static float smoothAxis2 = 16383.0;
    
    // Filtro EMA
    smoothAxis1 = (smoothAxis1 * 0.7) + (encoder3_pot.count * 0.3);
    smoothAxis2 = (smoothAxis2 * 0.7) + (encoder4_pot.count * 0.3);
    
    myData.axis1 = (int16_t)smoothAxis1;
    myData.axis2 = (int16_t)smoothAxis2;

    // --- 5. AGGIORNAMENTO SCHERMO HUD E STANDBY ---
    static int lastHUD_Axis1 = 16383;
    static int lastHUD_Axis2 = 16383;

    // Rileva i movimenti per lo schermo basandosi sul conto netto (prima dello smoothing)
    if (abs(encoder3_pot.count - lastHUD_Axis1) > 1500) { 
        lastActionText = "ASSE 1: " + String(encoder3_pot.count);
        lastHUD_Axis1 = encoder3_pot.count;
        userDidSomething = true;
    }
    if (abs(encoder4_pot.count - lastHUD_Axis2) > 1500) {
        lastActionText = "ASSE 2: " + String(encoder4_pot.count);
        lastHUD_Axis2 = encoder4_pot.count;
        userDidSomething = true;
    }

    if (myData.buttons != lastButtonsState) {
        for (int i = 0; i < 32; i++) {
            if (bitRead(myData.buttons, i) && !bitRead(lastButtonsState, i)) {
                if (i == 30 || i == 31) lastActionText = "ENC 1 (ROT)";
                else if (i == 28 || i == 29) lastActionText = "ENC 2 (ROT)";
                else if (i == 16) lastActionText = "ENC 1 (CLICK)";
                else if (i == 17) lastActionText = "ENC 2 (CLICK)";
                else if (i == 18) lastActionText = "ENC 3 (CLICK)";
                else if (i == 19) lastActionText = "ENC 4 (CLICK)";
                else lastActionText = "TASTO " + String(i + 1); 
                userDidSomething = true;
            }
        }
        lastButtonsState = myData.buttons;
    }

    if (userDidSomething) {
        lastActionTime = millis(); lastActivityTime = millis(); 
        if (!isScreenOn) { isScreenOn = true; showWelcomeMessage(); }
    }

    if (millis() - lastActionTime > 5000) lastActionText = ""; 
    
    if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
        turnOffScreen(); esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BTN_PIN, 0); esp_deep_sleep_start();
    }

    if (millis() - lastActivityTime > SCREEN_TIMEOUT && isScreenOn) { turnOffScreen(); isScreenOn = false; }

    if (millis() - lastBatCheck > 5000) { batteryPct = getBatteryPercentage(); lastBatCheck = millis(); }

    // --- 6. TRASMISSIONE REGOLATA ESP-NOW ---
    static unsigned long lastEspNowSend = 0;
    if (millis() - lastEspNowSend > 20) {
        esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        if (isScreenOn) updateHUD(batteryPct, masterConnected, lastActionText);
        lastEspNowSend = millis();
    }
}