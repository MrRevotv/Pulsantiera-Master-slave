# 🔌 Pinout e Cablaggi - ESP32-S3 Master

Questo documento descrive in dettaglio l'assegnazione dei pin per il modulo **Master** (ESP32-S3). Questo modulo agisce come ricevitore centrale (tramite ESP-NOW) e si interfaccia direttamente al PC simulando un singolo joystick USB, gestendo parallelamente i propri input hardware.

## 🎛️ Pulsanti Diretti (16 Tasti)

A differenza dei moduli Slave, il Master non utilizza una matrice di scansione ma legge gli input tramite **collegamenti diretti**. Tutti i pulsanti richiedono una chiusura verso GND (Massa) per generare il segnale.

| Componente | Pin ESP32-S3 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **Pulsanti 1-4** | `12`, `13`, `14`, `15` | `INPUT_PULLUP` | Collegare un polo al pin, l'altro a GND. |
| **Pulsanti 5-8** | `16`, `17`, `18`, `21` | `INPUT_PULLUP` | Collegare un polo al pin, l'altro a GND. |
| **Pulsanti 9-12**| `38`, `39`, `40`, `41` | `INPUT_PULLUP` | Collegare un polo al pin, l'altro a GND. |
| **Pulsanti 13-16**| `42`, `47`, `48`, `45` | `INPUT_PULLUP` | Collegare un polo al pin, l'altro a GND. |

---

## 🔄 Encoder 

Il Master gestisce 2 encoder rotativi standard per completare la dotazione del pannello.

### Encoder 1
| Segnale | Pin ESP32-S3 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **CLK (A)** | `4`  | `INPUT_PULLUP` | - |
| **DT (B)** | `5`  | `INPUT_PULLUP` | - |
| **SW (Click)**| `6`  | `INPUT_PULLUP` | Sfrutta la resistenza interna, chiude a GND. |

### Encoder 2
| Segnale | Pin ESP32-S3 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **CLK (A)** | `7`  | `INPUT_PULLUP` | - |
| **DT (B)** | `10` | `INPUT_PULLUP` | - |
| **SW (Click)**| `11` | `INPUT_PULLUP` | Sfrutta la resistenza interna, chiude a GND. |

---

## 🗺️ Diagramma di Collegamento Architetturale

```mermaid
graph TD
    %% Stili personalizzati
    classDef masterBoard fill:#2a2a2a,stroke:#e67e22,stroke-width:4px,color:#fff,rx:10px,ry:10px;
    classDef comms fill:#1abc9c,stroke:#16a085,stroke-width:2px,color:#fff,stroke-dasharray: 5 5;
    classDef usb fill:#8e44ad,stroke:#9b59b6,stroke-width:2px,color:#fff;
    classDef direct fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff;
    classDef encoder fill:#f39c12,stroke:#d35400,stroke-width:2px,color:#fff;

    ESP["👑 ESP32-S3 Master<br>(Hub Centrale)"]:::masterBoard

    subgraph Comunicazione ["🌐 Rete e Interfacce"]
        USB["💻 USB (SimGamepad)<br>Riconosciuto da Windows"]:::usb
        ESPN["📡 Ricevitore ESP-NOW<br>Riceve dati dagli Slave"]:::comms
    end
    ESP ==>|Cavo USB| USB
    ESPN -.->|Wireless| ESP

    subgraph Pulsanti ["🎛️ Input Diretti (16 Tasti)"]
        BTN1["Pulsanti 1-8<br>Pin: 12-18, 21"]:::direct
        BTN2["Pulsanti 9-16<br>Pin: 38-42, 45, 47, 48"]:::direct
    end
    ESP --> BTN1
    ESP --> BTN2

    subgraph Encoders ["🔄 Encoders Locali"]
        ENC1["Encoder 1<br>CLK: 4 | DT: 5 | SW: 6"]:::encoder
        ENC2["Encoder 2<br>CLK: 7 | DT: 10 | SW: 11"]:::encoder
    end
    ESP --> ENC1
    ESP --> ENC2
