# ✈️ Progetto Pulsantiera Master-Slave (Flight Sim Panel)

Benvenuto nel repository ufficiale del sistema di controllo custom per simulatori di volo. 
Questo progetto utilizza un'architettura **Master-Slave wireless** basata sul protocollo a bassissima latenza **ESP-NOW**.

L'obiettivo finale del sistema è gestire un totale di **96 pulsanti e 8 assi**, raggruppando tutti gli input fisici dei vari moduli periferici (Slave) per farli apparire a Windows come **un singolo joystick USB** tramite il modulo centrale (Master).

## 🛠️ Hardware Utilizzato

### 👑 Master (Hub Centrale e Interfaccia USB)
* **Scheda di sviluppo:** ESP32-S3 (AYWHP per ESP-DevKitC-S3)
* **Modulo MCU:** WROOM-1-N16R8 (a basso consumo)
* **Connettività:** Wi-Fi a doppia modalità e Bluetooth
* **Interfaccia:** Connettore USB Type-C (compatibile con ecosistema Arduino)
* **Ruolo:** Riceve i dati da tutti gli Slave via ESP-NOW, gestisce i propri input diretti (pulsanti ed encoder) e si interfaccia nativamente al PC come periferica di gioco USB (SimGamepad).

### 🖥️ Slave (Moduli Periferici)
* **Scheda di sviluppo:** ESP32 *[NOTA: Inserire qui il modello esatto della dev board]*
* **Dotazione hardware per ogni modulo:** 32 pulsanti (matrice), 4 encoder standard, 3 encoder-potenziometri (assi), display OLED per telemetria e un buzzer.
* **Ruolo:** Raccoglie gli input fisici, aggiorna il display locale e trasmette i pacchetti dati al Master ad alta frequenza (ogni 20ms).

---

## 📚 Documentazione e Pinout

Scegli quale modulo vuoi consultare per vedere gli schemi di cablaggio completi, i pinout e i diagrammi architetturali:

👉 **[Vai alla documentazione del Master (ESP32-S3)](./Master%20esp%2032/README.md)**

👉 **[Vai alla documentazione dello Slave 1 (ESP32)](./Slave%201%20ESP%2032/README.md)**

---

## 🚀 Istruzioni di Sviluppo (PlatformIO)

Questo progetto è strutturato per essere compilato tramite **PlatformIO** su Visual Studio Code.

1. Apri singolarmente la cartella del modulo su cui vuoi lavorare (`Master esp 32` oppure `Slave 1 ESP 32`).
2. Controlla il file `include/Config.h` di ogni modulo per verificare la corretta associazione dei pin e l'indirizzo MAC di destinazione per la rete ESP-NOW.
3. Collega la scheda tramite cavo USB dati.
4. Usa i comandi **Build** e **Upload** di PlatformIO per caricare il firmware.
