#ifndef __ARDUINO_H__
#define __ARDUINO_H__

#include <stdlib.h>
#include <string.h>
#include <spiclass.h>

typedef unsigned char uint8_t;
typedef uint8_t byte;
typedef unsigned int word;
#define PROGMEM
#define __FlashStringHelper char
#define F(text) text
byte pgm_read_byte(const byte* address_short);

//int digitalRead(uint8_t);
#define digitalRead(a) 1
//void digitalWrite(uint8_t, uint8_t);
#define digitalWrite(a, b)
//void pinMode(uint8_t, uint8_t);
#define pinMode(a, b)
//void delay(unsigned long us);
#define delay(us)
#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1

#define CHANGE 1
#define FALLING 2
#define RISING 3

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class SerialClass {
  public:
    void print(const char* s){};
    void print(int i, int f = DEC){};
    void println(const char* s){};
    void println(int i){};
    void println(void){};
};

extern SerialClass Serial;
extern SPIClass SPI;

#endif
