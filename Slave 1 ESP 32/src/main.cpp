#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Encoder.h>
#include <Keypad.h>
#include "DisplayManager.h" 

// --- MAC ADDRESS MASTER (Inserisci quello del tuo Master) ---
uint8_t broadcastAddress[] = {0xDC, 0xB4, 0xD9, 0x38, 0x20, 0x54}; 

// --- PIN SPECIALI ---
#define BATTERY_PIN 35
#define POWER_BTN_PIN 15 // Pin RTC per risvegliare dal Deep Sleep

// --- TIMERS ---
#define SCREEN_TIMEOUT 1200000 // 20 Minuti in millisecondi
#define SLEEP_TIMEOUT 3600000  // 1 Ora in millisecondi
bool isScreenOn = true;

// --- SETUP MATRICE 4x4 ---
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

// --- VARIABILI HARDWARE ---
ESP32Encoder encoder1;
ESP32Encoder encoder2;
ESP32Encoder encoder3_pot;
ESP32Encoder encoder4_pot;

long lastEnc1 = 0;
long lastEnc2 = 0;

// --- VARIABILI DI STATO E TEMPO ---
int batteryPct = 100;
unsigned long lastBatCheck = 0;
unsigned long lastActivityTime = 0; 
bool justWokeUp = true;

String lastActionText = "";
unsigned long lastActionTime = 0;
uint32_t lastButtonsState = 0;
int lastAxis1State = 16383;
int lastAxis2State = 16383;

// --- FUNZIONI ---

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  masterConnected = (status == ESP_NOW_SEND_SUCCESS);
}

int getBatteryPercentage() {
    int raw = analogRead(BATTERY_PIN);
    float voltage = (raw / 4095.0) * 2.0 * 3.3 * 1.1; 
    return constrain(map(voltage * 100, 330, 420, 0, 100), 0, 100);
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    initDisplay();
    drawBootSequence();

    pinMode(POWER_BTN_PIN, INPUT_PULLUP);

    ESP32Encoder::useInternalWeakPullResistors = UP;
    encoder1.attachHalfQuad(16, 5); 
    encoder2.attachHalfQuad(18, 17); 
    encoder3_pot.attachHalfQuad(21, 19); 
    encoder4_pot.attachHalfQuad(23, 22); 
    
    encoder3_pot.setCount(16383);
    encoder4_pot.setCount(16383);

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

// --- LOOP PRINCIPALE ---
void loop() {
    bool userDidSomething = false;

    // --- 0. GESTIONE PULSANTE DI ACCENSIONE / DEEP SLEEP ---
    bool powerBtnState = !digitalRead(POWER_BTN_PIN); 
    static unsigned long powerBtnPressTime = 0;
    static bool isPowerBtnHeld = false;

    if (powerBtnState) {
        if (!isPowerBtnHeld) {
            isPowerBtnHeld = true;
            powerBtnPressTime = millis();
        }
        
        unsigned long heldDuration = millis() - powerBtnPressTime;
        
        if (!isScreenOn) {
            isScreenOn = true;
            lastActivityTime = millis();
            showWelcomeMessage();
        }

        if (heldDuration > 1000) {
            int secondsLeft = 10 - ((heldDuration - 1000) / 1000);
            
            if (secondsLeft <= 0) {
                // TIMER SCADUTO
                showReleaseMsg(); // Funzione nel DisplayManager per "RILASCIA IL TASTO"

                while (!digitalRead(POWER_BTN_PIN)) {
                    delay(50);
                }
                
                delay(500);
                turnOffScreen(); // Spegne lo schermo tramite il manager

                esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BTN_PIN, 0); 
                esp_deep_sleep_start();
            } else {
                showPowerOffCountdown(secondsLeft);
                return; 
            }
        }
    } else {
        if (isPowerBtnHeld) {
            isPowerBtnHeld = false;
            lastActivityTime = millis(); 
        }
    }

    // --- 1. LETTURA INGRESSI ---
    if (matrixKeypad.getKeys()) {
        for (int i=0; i<LIST_MAX; i++) {
            if (matrixKeypad.key[i].stateChanged) {
                if (matrixKeypad.key[i].kstate == PRESSED || matrixKeypad.key[i].kstate == HOLD) {
                    bitSet(currentMatrixState, matrixKeypad.key[i].kchar);
                }
                if (matrixKeypad.key[i].kstate == RELEASED || matrixKeypad.key[i].kstate == IDLE) {
                    bitClear(currentMatrixState, matrixKeypad.key[i].kchar);
                }
            }
        }
    }
    myData.buttons = currentMatrixState;

    long newEnc1 = encoder1.getCount() / 2;
    if (newEnc1 > lastEnc1) { bitSet(myData.buttons, 30); } 
    if (newEnc1 < lastEnc1) { bitSet(myData.buttons, 31); } 
    lastEnc1 = newEnc1;

    long newEnc2 = encoder2.getCount() / 2;
    if (newEnc2 > lastEnc2) { bitSet(myData.buttons, 28); } 
    if (newEnc2 < lastEnc2) { bitSet(myData.buttons, 29); } 
    lastEnc2 = newEnc2;

    int rawAxis1 = encoder3_pot.getCount() * 500; 
    int rawAxis2 = encoder4_pot.getCount() * 500;
    
    myData.axis1 = constrain(rawAxis1, 0, 32767);
    myData.axis2 = constrain(rawAxis2, 0, 32767);
    
    if (rawAxis1 > 32767) encoder3_pot.setCount(32767 / 500);
    if (rawAxis1 < 0) encoder3_pot.setCount(0);
    if (rawAxis2 > 32767) encoder4_pot.setCount(32767 / 500);
    if (rawAxis2 < 0) encoder4_pot.setCount(0);

    // --- 2. RILEVAMENTO INPUT PER HUD ---
    if (abs(myData.axis1 - lastAxis1State) > 500) { 
        lastActionText = "ASSE 1: " + String(myData.axis1);
        lastAxis1State = myData.axis1;
        userDidSomething = true;
    }
    
    if (abs(myData.axis2 - lastAxis2State) > 500) {
        lastActionText = "ASSE 2: " + String(myData.axis2);
        lastAxis2State = myData.axis2;
        userDidSomething = true;
    }

    if (myData.buttons != lastButtonsState) {
        for (int i = 0; i < 32; i++) {
            if (bitRead(myData.buttons, i) && !bitRead(lastButtonsState, i)) {
                if (i == 30 || i == 31) lastActionText = "ENC 1 (ROT)";
                else if (i == 28 || i == 29) lastActionText = "ENC 2 (ROT)";
                else lastActionText = "TASTO " + String(i + 1); 
                userDidSomething = true;
            }
        }
        lastButtonsState = myData.buttons;
    }

    // --- 3. GESTIONE STANDBY E SLEEP ---
    if (userDidSomething) {
        lastActionTime = millis();
        lastActivityTime = millis(); 
        if (!isScreenOn) {
            isScreenOn = true;
            showWelcomeMessage();
        }
    }

    if (millis() - lastActionTime > 5000) {
        lastActionText = ""; 
    }

    // Deep Sleep automatico (1 ora)
    if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
        turnOffScreen();
        esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BTN_PIN, 0); 
        esp_deep_sleep_start();
    }

    // Standby schermo (20 minuti)
    if (millis() - lastActivityTime > SCREEN_TIMEOUT) {
        if (isScreenOn) {
            turnOffScreen();      
            isScreenOn = false;     
        }
    }

    if (millis() - lastBatCheck > 5000) {
        batteryPct = getBatteryPercentage();
        lastBatCheck = millis();
    }

    // --- 4. TRASMISSIONE E HUD ---
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    if (isScreenOn) {
        updateHUD(batteryPct, masterConnected, lastActionText);
    }

    delay(20); 
}