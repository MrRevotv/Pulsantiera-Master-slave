#ifndef CUSTOM_ENCODER_H
#define CUSTOM_ENCODER_H

#include <Arduino.h>

// Definizioni per la direzione estratte dal tuo codice
#define DIR_CCW 0x10
#define DIR_CW  0x20
#define R_START 0x0

class CustomEncoder {
public:
    // isPotentiometer = true se l'encoder deve comportarsi come un asse
    CustomEncoder(uint8_t pinA, uint8_t pinB, bool isPotentiometer = false);
    
    void begin();
    uint8_t process(); // Da chiamare costantemente nel loop
    
    int32_t count; // Usato solo se isPotentiometer = true

private:
    uint8_t _pinA;
    uint8_t _pinB;
    bool _isPot;
    unsigned char _state;
};

#endif