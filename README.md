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
| **Riga 1** | `32` | Collegare al pin Row 1 della matrice. |
| **Riga 2** | `33` | Collegare al pin Row 2 della matrice. |
| **Riga 3** | `25` | Collegare al pin Row 3 della matrice. |
| **Riga 4** | `14` | Collegare al pin Row 4 della matrice. |
| **Colonna 1** | `13` | Collegare al pin Col 1 della matrice. |
| **Colonna 2** | `12` | Collegare al pin Col 2 della matrice. |
| **Colonna 3** | `4`  | Collegare al pin Col 3 della matrice. |
| **Colonna 4** | `2`  | Collegare al pin Col 4 della matrice. |

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
| **CLK (A)** | `1`  | `INPUT_PULLUP` | (Attenzione: Pin usato dal Serial TX, ma disconnesso nel setup attuale) |
| **DT (B)** | `3`  | `INPUT_PULLUP` | (Attenzione: Pin usato dal Serial RX, ma disconnesso nel setup attuale) |
| **SW (Click)** | `0`  | `INPUT_PULLUP` | Possiede pull-up interno. Collegare a GND per il click. |

---

## 🗺️ Diagramma di Collegamento

```mermaid
graph TD
    ESP[**ESP32 Slave 1**<br/>Modulo Centale]

    %% Alimentazione
    subgraph Alimentazione e Sistema
        ESP ---|Pin 35| BAT[Lettura Sensore Batteria Lipo]
        ESP ---|Pin 15| PWR[Pulsante Power]
    end

    %% Display
    subgraph Display I2C
        ESP ---|Pin 26| SDA[OLED SDA]
        ESP ---|Pin 27| SCL[OLED SCL]
    end

    %% Matrice
    subgraph Matrice Tasti 4x4
        ESP ---|32, 33, 25, 14| ROWS[Righe 1-4]
        ESP ---|13, 12, 4, 2| COLS[Colonne 1-4]
    end

    %% Encoders
    subgraph Hardware Esterno Sensibile
        ESP ---|5, 18| ENC1[Encoder 1 Dati]
        ESP ---|34| ENC1SW[Encoder 1 Click *]
        
        ESP ---|19, 21| ENC2[Encoder 2 Dati]
        ESP ---|36| ENC2SW[Encoder 2 Click *]
        
        ESP ---|22, 23| ENC3[Encoder 3 Dati]
        ESP ---|39| ENC3SW[Encoder 3 Click *]
        
        ESP ---|1, 3| ENC4[Encoder 4 Dati]
        ESP ---|0| ENC4SW[Encoder 4 Click]
    end
    
    classDef warning fill:#f9d0c4,stroke:#333,stroke-width:2px;
    class ENC1SW,ENC2SW,ENC3SW warning;
