# 🔌 Pinout e Cablaggi - ESP32 Slave 1

Questo documento descrive in dettaglio l'assegnazione dei pin per il modulo **Slave 1** (ESP32). Rispettare rigorosamente questo schema durante le saldature per garantire il corretto funzionamento del firmware, delle letture hardware e della comunicazione ESP-NOW.

## 🔋 Sistema e Alimentazione

| Componente | Pin ESP32 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **Pulsante Power / Wake** | `15` | `INPUT_PULLUP` | Chiude a GND (Massa) per l'accensione e lo spegnimento. |
| **Lettura Voltaggio Lipo** | `35` | `ANALOG (ADC)` | **Non collegare nulla.** Il pin è collegato internamente al partitore di tensione del connettore JST della batteria. |

---

## 📺 Display OLED (I2C)

| Componente | Pin ESP32 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **SDA (Dati)** | `26` | `I2C` | Comunicazione con il display SH110X. |
| **SCL (Clock)** | `27` | `I2C` | Comunicazione con il display SH110X. |

---

## 🎛️ Matrice Pulsanti (4x4)

La libreria gestisce automaticamente le logiche di pull-up per la scansione della matrice.

| Componente | Pin ESP32 | Note |
| :--- | :---: | :--- |
| **Righe 1-4** | `32`, `33`, `25`, `14` | Collegare in ordine ai pin Row della matrice. |
| **Colonne 1-4** | `13`, `12`, `4`, `2` | Collegare in ordine ai pin Col della matrice. |

---

## 🔄 Encoder e Potenziometri (Assi)

⚠️ **ATTENZIONE HARDWARE:** I pin `34`, `36` e `39` dell'ESP32 sono *Input-Only* e non possiedono resistenze di pull-up interne. I pulsanti degli Encoder 1, 2 e 3 necessitano obbligatoriamente di una **resistenza di pull-up esterna (es. 10kΩ)** collegata a 3.3V oppure dei moduli ky-040, altrimenti si verificheranno false pressioni (floating pins / crosstalk); ATTENZIONE - Molti rivenditori dicono di avere il modulo ky-040 ma in verità sono cloni a basso costo hw-040 ai quali manca la terza resistenza (appunto quella del pulsante).

### Encoder 1 (Standard)
| Segnale | Pin ESP32 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **CLK (A)** | `5`  | `INPUT_PULLUP` | - |
| **DT (B)** | `18` | `INPUT_PULLUP` | - |
| **SW (Click)** | `34` | `INPUT` | ⚠️ **Richiede resistenza esterna a 3.3V** |

### Encoder 2 (Standard)
| Segnale | Pin ESP32 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **CLK (A)** | `19` | `INPUT_PULLUP` | - |
| **DT (B)** | `21` | `INPUT_PULLUP` | - |
| **SW (Click)** | `36` | `INPUT` | ⚠️ **Richiede resistenza esterna a 3.3V** |

### Encoder 3 (Potenziometro 1 - Asse 1)
| Segnale | Pin ESP32 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **CLK (A)** | `22` | `INPUT_PULLUP` | - |
| **DT (B)** | `23` | `INPUT_PULLUP` | - |
| **SW (Click)** | `39` | `INPUT` | ⚠️ **Richiede resistenza esterna a 3.3V** |

### Encoder 4 (Potenziometro 2 - Asse 2)
| Segnale | Pin ESP32 | Tipo | Note |
| :--- | :---: | :---: | :--- |
| **CLK (A)** | `1`  | `INPUT_PULLUP` | (Pin usato dal Serial TX, disconnesso nel setup attuale) |
| **DT (B)** | `3`  | `INPUT_PULLUP` | (Pin usato dal Serial RX, disconnesso nel setup attuale) |
| **SW (Click)** | `0`  | `INPUT_PULLUP` | Possiede pull-up interno. Collegare a GND per il click. |

---

## 🗺️ Diagramma di Collegamento Architetturale

```mermaid
graph TD
    %% Stili personalizzati
    classDef mainBoard fill:#2a2a2a,stroke:#4caf50,stroke-width:4px,color:#fff,rx:10px,ry:10px;
    classDef pwr fill:#e74c3c,stroke:#c0392b,stroke-width:2px,color:#fff;
    classDef i2c fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff;
    classDef matrix fill:#9b59b6,stroke:#8e44ad,stroke-width:2px,color:#fff;
    classDef encoder fill:#f39c12,stroke:#d35400,stroke-width:2px,color:#fff;
    classDef warning fill:#f1c40f,stroke:#e67e22,stroke-width:3px,color:#333;
    classDef comms fill:#1abc9c,stroke:#16a085,stroke-width:2px,color:#fff,stroke-dasharray: 5 5;

    ESP["🖥️ ESP32 Slave 1<br>(Modulo Periferico)"]:::mainBoard

    subgraph Alimentazione ["⚡ Alimentazione e Power"]
        BAT["🔋 Sensore Lipo (Pin 35)"]:::pwr
        PWR["🔘 Pulsante Power (Pin 15)"]:::pwr
    end
    ESP --> BAT
    ESP --> PWR

    subgraph Display ["📺 Display OLED (I2C)"]
        SDA["SDA (Pin 26)"]:::i2c
        SCL["SCL (Pin 27)"]:::i2c
    end
    ESP --> SDA
    ESP --> SCL

    subgraph Matrice ["🎛️ Matrice Tasti (4x4)"]
        ROWS["Righe: 32, 33, 25, 14"]:::matrix
        COLS["Colonne: 13, 12, 4, 2"]:::matrix
    end
    ESP --> ROWS
    ESP --> COLS

    subgraph Encoders ["🔄 Hardware Esterno Sensibile"]
        ENC1["Encoder 1<br>CLK: 5 | DT: 18"]:::encoder
        ENC1_SW["⚠️ SW: 34<br>(Pull-up esterno)"]:::warning

        ENC2["Encoder 2<br>CLK: 19 | DT: 21"]:::encoder
        ENC2_SW["⚠️ SW: 36<br>(Pull-up esterno)"]:::warning

        ENC3["Encoder 3 (Asse 1)<br>CLK: 22 | DT: 23"]:::encoder
        ENC3_SW["⚠️ SW: 39<br>(Pull-up esterno)"]:::warning

        ENC4["Encoder 4 (Asse 2)<br>CLK: 1 | DT: 3"]:::encoder
        ENC4_SW["SW: 0<br>(Pull-up interno)"]:::encoder
    end

    ESP --> ENC1
    ESP --> ENC1_SW
    ESP --> ENC2
    ESP --> ENC2_SW
    ESP --> ENC3
    ESP --> ENC3_SW
    ESP --> ENC4
    ESP --> ENC4_SW
    
    %% Rete
    ESP -.->|"Invia Dati (Ogni 20ms)"| ESPN["📡 ESP-NOW (Verso il Master)"]:::comms