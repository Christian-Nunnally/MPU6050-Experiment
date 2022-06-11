#include "color_utilities.h"

uint8_t get_red(uint32_t color)
{
  return (color >> 16) & 0xFF;
}

uint8_t get_green(uint32_t color)
{
  return (color >> 8) & 0xFF;
}

uint8_t get_blue(uint32_t color)
{
  return color & 0xFF;
}

uint32_t get_color_from_wheel(byte wheel_byte, Adafruit_NeoPixel *pixels)
{
  byte red, green, blue;
  if(wheel_byte < 85)
  {
    red = 255 - wheel_byte * 3;
    blue = wheel_byte * 3;
  }
  else if(wheel_byte < 170)
  {
    wheel_byte -= 85;
    green = wheel_byte * 3;
    blue = 255 - wheel_byte * 3;
  }
  else
  {
    wheel_byte -= 170;
    red = wheel_byte * 3;
    green = 255 - wheel_byte * 3;
  }
  return pixels->Color(red, green, blue);
}
