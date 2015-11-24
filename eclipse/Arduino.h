#ifndef __ARDUINO_H__
#define __ARDUINO_H__

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <spiclass.h>

typedef unsigned char uint8_t;
typedef uint8_t byte;
typedef unsigned int word;
#define PROGMEM
#define __FlashStringHelper char
#define F(text) text
byte pgm_read_byte(const byte* address_short);

int digitalRead(uint8_t);
void digitalWrite(uint8_t, uint8_t);
void pinMode(uint8_t, uint8_t);
#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1

#define CHANGE 1
#define FALLING 2
#define RISING 3

void delay(unsigned long);

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class SerialClass {
  public:
    void print(const char* s){std::cout << s;};
    void print(int i, int f = DEC){std::cout << i;};
    void println(const char* s){std::cout << s << std::endl;};
    void println(int i){std::cout << i << std::endl;};
    void println(void){std::cout << std::endl;};
};

extern SerialClass Serial;
extern SPIClass SPI;

#endif
