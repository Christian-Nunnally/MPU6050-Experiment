#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "color_utilities.h"

#define IO_PIN_1 4
#define IO_PIN_2 2
#define IO_PIN_3 29
#define IO_PIN_4 28
#define LED_PIN 1
#define IO_5V_PIN 3
#define IO_GROUND_PIN 27
#define POTENTIOMETER_PIN A0

#define GYRO_X_OFFSET 0.04
#define GYRO_Y_OFFSET 0.00
#define GYRO_Z_OFFSET 0.01
#define ACCELERATION_X_OFFSET -0.35
#define ACCELERATION_Y_OFFSET 0.18
#define ACCELERATION_Z_OFFSET 0.07
#define ACCELERATION_SMOOTHING .5
#define GYRO_SMOOTHING .5

#define LEDS_PER_RING 36
#define LED_COUNT LEDS_PER_RING * 2
#define POTENTIOMETER_MAX 1024
#define MAX_BALLS 5
#define MAX_BALL_SPIN_SPEED 3
#define THRUST_UPPER_THRESHOLD_SQUARED 100
#define THRUST_LOWER_THRESHOLD_SQUARED 9

#define MAX_FADE_SPEED 10
#define PATTERN_LENGTH 9
#define PATTERN_COUNT 16
#define PATTERN_LENGTH 18
#define FRAME_DELAY_MILLISECONDS 13

Adafruit_MPU6050 mpu;
Adafruit_NeoPixel pixels(LED_COUNT + 1, LED_PIN, NEO_GRB + NEO_KHZ800);

int io_state_1 = 0;
int io_state_2 = 0;
int io_state_3 = 0;
int io_state_4 = 0;
float brightnessValue = 0;
float accelerationX = 0;
float accelerationY = 0;
float accelerationZ = 0;
float gyroX = 0;
float gyroY = 0;
float gyroZ = 0;
byte redFadeAmount = 0;
byte greenFadeAmount = 0;
byte blueFadeAmount = 0;
bool flipper;
bool thrustToggle;
int globalPatternIndex;
float globalHue;
bool rainbowMode;
bool tertiaryMode;
bool spazMode;
int timeUntilSleep;

float oldBallPositions[MAX_BALLS];
float ballPositions[MAX_BALLS];
float ballVelocities[MAX_BALLS];
float ballFactors[MAX_BALLS];
int numberOfBalls = 1;
int last_io_1_value;

uint32_t colors[LED_COUNT];
uint32_t palletColors[4] = {0,0,0,0};
int colorMode;
byte pattern[PATTERN_LENGTH] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
byte patterns[4][PATTERN_COUNT][PATTERN_LENGTH] = 
{
  {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,1,1,1,3,1,2,1,1,1,1,1,1,1,3,1,1},
    {1,1,1,2,2,2,1,1,1,2,2,2,1,1,1,2,2,2},
    {1,1,1,2,2,2,1,1,1,2,2,2,1,1,1,2,2,2},
    {1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1},
    {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
    {2,2,1,1,2,2,1,1,1,1,1,1,2,2,1,1,2,2},
    {1,2,3,3,2,1,1,2,3,3,2,1,1,2,3,3,2,1},
    {1,1,3,3,1,1,2,2,1,1,3,3,1,1,2,2,1,1},
    {1,1,1,2,2,2,1,1,1,3,3,3,1,1,1,2,2,2},
    {1,1,1,3,3,3,2,2,2,2,2,2,3,3,3,1,1,1},
    {1,1,1,2,2,2,3,3,3,1,1,1,2,2,2,3,3,3},
    {1,2,1,3,3,1,2,1,3,1,2,3,2,1,3,1,2,1},
    {1,2,1,2,1,2,2,1,2,1,2,1,1,3,1,3,1,3},
    {3,2,1,3,2,1,3,2,1,1,2,3,1,2,3,1,2,3},
    {3,2,1,1,2,2,1,3,1,2,3,2,1,1,2,2,1,3},
  },
  {
    {1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0},
    {1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0},
    {3,3,3,2,2,2,1,1,1,0,0,0,1,1,1,2,2,2},
    {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0},
    {2,1,2,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0},
    {1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
    {1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
    {3,0,3,0,3,0,0,0,1,1,1,0,0,0,0,0,0,0},
    {1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,2,3,0,0,0,3,2,1,0,0,0,0,0,0,0,0,0},
    {3,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0},
    {3,0,2,0,3,0,2,0,3,0,2,0,3,0,2,0,3,0},
    {1,0,1,0,1,0,1,0,1,2,0,2,0,2,0,2,0,2},
  },
  {
    {3,2,1,1,2,2,1,3,1,2,3,2,1,1,2,2,1,3},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,1,1,1,3,1,2,1,1,1,1,1,1,1,3,1,1},
    {1,1,1,2,2,2,1,1,1,2,2,2,1,1,1,2,2,2},
    {1,1,1,2,2,2,1,1,1,2,2,2,1,1,1,2,2,2},
    {1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1},
    {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
    {2,2,1,1,2,2,1,1,1,1,1,1,2,2,1,1,2,2},
    {1,2,3,3,2,1,1,2,3,3,2,1,1,2,3,3,2,1},
    {1,1,3,3,1,1,2,2,1,1,3,3,1,1,2,2,1,1},
    {1,1,1,2,2,2,1,1,1,3,3,3,1,1,1,2,2,2},
    {1,1,1,3,3,3,2,2,2,2,2,2,3,3,3,1,1,1},
    {1,1,1,2,2,2,3,3,3,1,1,1,2,2,2,3,3,3},
    {1,2,1,3,3,1,2,1,3,1,2,3,2,1,3,1,2,1},
    {1,2,1,2,1,2,2,1,2,1,2,1,1,3,1,3,1,3},
    {3,2,1,3,2,1,3,2,1,1,2,3,1,2,3,1,2,3},
  },
  {
    {3,2,1,3,2,1,3,2,1,1,2,3,1,2,3,1,2,3},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,1,1,1,3,1,2,1,1,1,1,1,1,1,3,1,1},
    {1,1,1,2,2,2,1,1,1,2,2,2,1,1,1,2,2,2},
    {1,1,1,2,2,2,1,1,1,2,2,2,1,1,1,2,2,2},
    {1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1},
    {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
    {2,2,1,1,2,2,1,1,1,1,1,1,2,2,1,1,2,2},
    {1,1,3,3,1,1,2,2,1,1,3,3,1,1,2,2,1,1},
    {1,1,1,2,2,2,1,1,1,3,3,3,1,1,1,2,2,2},
    {1,1,1,3,3,3,2,2,2,2,2,2,3,3,3,1,1,1},
    {1,1,1,2,2,2,3,3,3,1,1,1,2,2,2,3,3,3},
    {1,2,1,3,3,1,2,1,3,1,2,3,2,1,3,1,2,1},
    {1,2,1,2,1,2,2,1,2,1,2,1,1,3,1,3,1,3},
    {1,2,3,3,2,1,1,2,3,3,2,1,1,2,3,3,2,1},
  },
};

void setup() {
  setup_pins();
  setup_accelerometer();
  spin_balls(MAX_BALL_SPIN_SPEED);
}

void setup_pins()
{
  pixels.begin();
  pinMode(IO_5V_PIN, OUTPUT);
  pinMode(IO_GROUND_PIN, OUTPUT);
  pinMode(IO_PIN_1, INPUT);
  pinMode(IO_PIN_2, INPUT);
  pinMode(IO_PIN_3, INPUT);
  pinMode(IO_PIN_4, INPUT);
  digitalWrite(IO_5V_PIN, HIGH);
  digitalWrite(IO_GROUND_PIN, LOW);
}

void setup_accelerometer()
{
  while (!mpu.begin()) delay(100);
  mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_10_HZ);
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
}

void loop() 
{
  read_mpu6050();
  read_inputs();
  for (int i = 0; i < numberOfBalls; i++)
  {
    float amount = min(max(gyroX, -1), 1);
    amount /= 8;
    ballVelocities[i] += amount * ballFactors[i];
    ballPositions[i] += ballVelocities[i];
    move_ball(i, ballVelocities[i]);
    ballVelocities[i] *= .95;
  }
  detect_bumps();
  fade_pixels();
  adjust_hue();
  int allowedPatterns = 1;
  if (io_state_2) allowedPatterns = 4;
  if (io_state_4) randomize(io_state_3);
  if (io_state_3 && random(80) == 0) randomize_pattern(globalPatternIndex, random(PATTERN_LENGTH), random(allowedPatterns));
  if (last_io_1_value != io_state_1)
  {
    last_io_1_value = io_state_1;
    numberOfBalls = (numberOfBalls + 1) % MAX_BALLS;
  }
  show_frame();
}

void adjust_hue()
{
  if (spazMode && rainbowMode) set_colors(globalHue + 1 % 255, !tertiaryMode);
  else if (rainbowMode) set_colors(globalHue + 1 % 255, tertiaryMode);
  else if (spazMode) set_colors(globalHue, !tertiaryMode);
  bump_hue_with_y_acceleration();
}

void bump_hue_with_y_acceleration()
{
  globalHue = min(255, max(0, globalHue + accelerationY));
}

void show_frame()
{
  pixels.show();
  delay(FRAME_DELAY_MILLISECONDS);
}

void randomize(bool useAlternatePatterns)
{
  if (random(50) == 0) randomize_fade();
  if (random(140) == 0) randomize_colors();
  if (useAlternatePatterns && random(100) == 0) randomize_pattern(globalPatternIndex, random(PATTERN_LENGTH), random(4));
  if (random(120) == 0) randomize_pattern(random(PATTERN_COUNT), random(PATTERN_LENGTH), 0);
}

void switch_mode()
{
  randomize_fade();
  randomize_colors();
  randomize_pattern(random(PATTERN_COUNT), random(PATTERN_LENGTH), random(4));
}

void randomize_colors() 
{
  rainbowMode = random(5) == 0;
  tertiaryMode = random(2) == 0;
  set_colors(random(255), tertiaryMode);
}

void set_colors(int hue, bool trinaryColor)
{
  globalHue = hue;
  palletColors[1] = get_color_from_wheel(hue, &pixels);
  if (!trinaryColor)
  {
    palletColors[2] = get_color_from_wheel((byte)(hue + 118 + random(20) % 255), &pixels);
    palletColors[3] = pixels.Color(0,0,0);
  }
  else
  {
    palletColors[2] = get_color_from_wheel((byte)(hue + 75 + random(20) % 255), &pixels);
    palletColors[3] = get_color_from_wheel((byte)(hue + 160 + random(20) % 255), &pixels);
  }
}

void randomize_pattern(int patternIndex, int patternOffset, int alternatePattern) 
{
  globalPatternIndex = patternIndex;
  for (int i = 0; i < PATTERN_LENGTH; i++)
  {
    pattern[i] = patterns[alternatePattern][patternIndex][i + patternOffset % PATTERN_LENGTH];
  }
}

void randomize_fade()
{
  redFadeAmount = random(MAX_FADE_SPEED);
  greenFadeAmount = random(MAX_FADE_SPEED);
  blueFadeAmount = random(MAX_FADE_SPEED);
}

void move_ball(int ballIndex, float amount)
{
  ballPositions[ballIndex] += amount;
  if (ballPositions[ballIndex] < 0) ballPositions[ballIndex] = LED_COUNT - 1;
  else if (ballPositions[ballIndex] >= LED_COUNT) ballPositions[ballIndex] = 0;
  if (ballVelocities[ballIndex] > 0)
  {
    for (int i = oldBallPositions[ballIndex]; ; i++)
    {
      if (i >= LED_COUNT)i = 0;
      uint32_t color = palletColors[pattern[i % PATTERN_LENGTH]];
      set_pixel(i, color);
      if (i == (int)ballPositions[ballIndex]) break;
    }
  }
  else
  {
    for (int i = oldBallPositions[ballIndex]; ; i--)
    {
      if (i < 0) i = LED_COUNT - 1;
      uint32_t color = palletColors[pattern[i % PATTERN_LENGTH]];
      set_pixel(i, color);
      if (i == (int)ballPositions[ballIndex]) break;
    }
  }
  oldBallPositions[ballIndex] = ballPositions[ballIndex];
}

void detect_bumps()
{
  if (flipper && accelerationX > 14) 
  {
    flipper = !flipper;
    flash_current_pattern();
    switch_mode();
    spazMode = false;
    spin_balls(MAX_BALL_SPIN_SPEED);
  }
  else if (!flipper && accelerationX < 0) 
  {
    flipper = !flipper;
    switch_mode();
    flash_current_pattern();
    switch_mode();
    spazMode = true;
  }
}

void spin_balls(int max_speed)
{
  for (int i = 0; i < MAX_BALLS; i++)
  {
    ballVelocities[i] = random(max_speed * 2) - max_speed;
    ballFactors[i] = (random(1001) / 1000.0) * 2 - 1;
  }
}

void flash_current_pattern()
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    uint32_t color = palletColors[pattern[i % PATTERN_LENGTH]];
    set_pixel(i, color);
  }
}

void fade_pixels()
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    uint32_t color = get_pixel(i);
    if (random(4) == 0)
    {
      uint8_t red = max(0, get_red(color) - redFadeAmount - random(2));
      uint8_t green = max(0, get_green(color) - greenFadeAmount - random(2));
      uint8_t blue = max(0, get_blue(color) - blueFadeAmount - random(2));
      set_pixel(i, pixels.Color(red, green, blue));
    }
    else 
    {
      uint8_t red = max(0, get_red(color) - redFadeAmount);
      uint8_t green = max(0, get_green(color) - greenFadeAmount);
      uint8_t blue = max(0, get_blue(color) - blueFadeAmount);
      set_pixel(i, pixels.Color(red, green, blue));
    }
  }
}

void set_pixel(int index, uint32_t color)
{
  colors[index] = color;
  if (index % 2 == 0) index /= 2;
  else index = (index - 1) / 2 + LEDS_PER_RING;
  pixels.setPixelColor(++index, color);
}

int get_pixel(int index)
{
  return colors[index];
}

void read_mpu6050()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  accelerationX = (a.acceleration.x + ACCELERATION_X_OFFSET) * (1 - ACCELERATION_SMOOTHING) + accelerationX * ACCELERATION_SMOOTHING;
  accelerationY = (a.acceleration.y + ACCELERATION_Y_OFFSET) * (1 - ACCELERATION_SMOOTHING) + accelerationY * ACCELERATION_SMOOTHING;
  accelerationZ = (a.acceleration.z + ACCELERATION_Z_OFFSET) * (1 - ACCELERATION_SMOOTHING) + accelerationZ * ACCELERATION_SMOOTHING;
  gyroX = (g.gyro.x + GYRO_X_OFFSET) * (1 - GYRO_SMOOTHING) + gyroX * GYRO_SMOOTHING;
  gyroY = (g.gyro.y + GYRO_Y_OFFSET) * (1 - GYRO_SMOOTHING) + gyroY * GYRO_SMOOTHING;
  gyroZ = (g.gyro.z + GYRO_Z_OFFSET) * (1 - GYRO_SMOOTHING) + gyroZ * GYRO_SMOOTHING;
}

void read_inputs()
{
  brightnessValue = analogRead(POTENTIOMETER_PIN) / POTENTIOMETER_MAX;
  io_state_1 = digitalRead(IO_PIN_1);
  io_state_2 = digitalRead(IO_PIN_2);
  io_state_3 = digitalRead(IO_PIN_3);
  io_state_4 = digitalRead(IO_PIN_4);
}
