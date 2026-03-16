#include "CustomEncoder.h"

#define HALF_STEP // Usiamo l'Half Step come nel tuo codice di esempio

#ifdef HALF_STEP
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const unsigned char ttable[6][4] = {
  { R_START_M,           R_CW_BEGIN,     R_CCW_BEGIN,  R_START },
  { R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START },
  { R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START },
  { R_START_M,           R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START },
  { R_START_M,           R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW },
  { R_START_M,           R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW },
};
#else
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6
const unsigned char ttable[7][4] = {
  { R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START },
  { R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW },
  { R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START },
  { R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START },
  { R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START },
  { R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW },
  { R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START },
};
#endif

CustomEncoder::CustomEncoder(uint8_t pinA, uint8_t pinB, bool isPotentiometer) {
    _pinA = pinA;
    _pinB = pinB;
    _isPot = isPotentiometer;
    count = 16383; // Valore centrale di partenza per gli assi
    _state = R_START;
}

void CustomEncoder::begin() {
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);
}

uint8_t CustomEncoder::process() {
    // Legge i due pin e li unisce in un singolo byte (0 a 3)
    unsigned char pinstate = (digitalRead(_pinB) << 1) | digitalRead(_pinA);
    
    // Controlla la matrice ttable per vedere in che stato siamo
    _state = ttable[_state & 0xf][pinstate];
    
    // Estrae solo i bit di direzione
    uint8_t result = _state & 0x30;
    
    // Se è un potenziometro virtuale, aggiorna autonomamente il contatore
    if (_isPot) {
        if (result == DIR_CW) {
            count += 500;
        } else if (result == DIR_CCW) {
            count -= 500;
        }
        
        // Limita i valori tra 0 e 32767
        if (count > 32767) count = 32767;
        if (count < 0) count = 0;
    }
    
    return result; // Ritorna DIR_CW, DIR_CCW o R_START (nessun movimento)
}