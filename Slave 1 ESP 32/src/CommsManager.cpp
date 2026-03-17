#include "CommsManager.h"

// Variabile globale necessaria per la callback di ESP-NOW
volatile bool isMasterConnected = false;

// Funzione di Callback chiamata automaticamente da ESP-NOW quando invia un pacchetto
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    isMasterConnected = (status == ESP_NOW_SEND_SUCCESS);
}

namespace CommsManager {
    
    esp_now_peer_info_t peerInfo;
    unsigned long lastEspNowSend = 0;
    uint8_t targetMac[6];

    void init(const uint8_t* masterMac) {
        // Salva il MAC Address localmente
        memcpy(targetMac, masterMac, 6);

        // Accendi il WiFi in modalità Stazione
        WiFi.mode(WIFI_STA);
        
        // Inizializza ESP-NOW
        if (esp_now_init() == ESP_OK) {
            esp_now_register_send_cb(OnDataSent);
            
            // Configura il Master come Peer (destinatario)
            memcpy(peerInfo.peer_addr, targetMac, 6);
            peerInfo.channel = 0;  
            peerInfo.encrypt = false;
            
            // Aggiungi il Peer
            esp_now_add_peer(&peerInfo);
        }
    }

    bool isConnected() {
        return isMasterConnected;
    }

    void sendIfReady(struct_message& dataToSend) {
        // Invia i dati massimo ogni tot millisecondi (definiti in Config.h) 
        // per non intasare l'antenna e lasciare tempo di calcolo agli encoder
        if (millis() - lastEspNowSend > ESPNOW_SEND_RATE) {
            esp_now_send(targetMac, (uint8_t *) &dataToSend, sizeof(dataToSend));
            lastEspNowSend = millis();
        }
    }
}