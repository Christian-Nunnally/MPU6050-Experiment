#ifndef COLOR_UTILITIES
#define COLOR_UTILITIES
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

uint8_t get_red(uint32_t color);
uint8_t get_green(uint32_t color);
uint8_t get_blue(uint32_t color);
uint32_t get_color_from_wheel(byte wheel_byte, Adafruit_NeoPixel *pixels);

#endif
