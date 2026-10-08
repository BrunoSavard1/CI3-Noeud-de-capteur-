// Libraries
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_VEML7700.h"
#include "ScioSense_ENS160.h"  // ENS160 library
#include <FastLED.h>


// Defines
#define SHT41_ADDR 0X44 // REGISTER ADDR
#define HIGHR_ADDR 0XFD // HIGH REPEATABILITY REGISTER ADDR
#define NUM_LEDS 1
#define NEO_PIN 3

#define LEDCHARGE 10
#define LEDBAT 11
#define LEDFULL 12

// Variables and objects
Adafruit_VEML7700 veml = Adafruit_VEML7700();
ScioSense_ENS160      ens160(ENS160_I2CADDR_0);
CRGB leds[NUM_LEDS]; // for fastLED

void convertSHT41(uint16_t SHT_SRH, uint16_t SHT_ST)
{
  float SHT_RH   = -6.0f   + 125.0f * ((float)SHT_SRH / 65535.0f); // in %
  float SHT_Temp = -45.0f  + 175.0f * ((float)SHT_ST  / 65535.0f); // in °C

  Serial.print("SHT_RH: ");
  Serial.println(SHT_RH);

  Serial.print("SHT_Temp: ");
  Serial.println(SHT_Temp);
}

void setup() {
  Wire.begin();        // I2C Init
  Serial.begin(9600);  // SERIAL for communication

  //#################################################VEML7700#################################################

  if (!veml.begin()) {
    Serial.println("VEML7700 not found!");
  }
  veml.setGain(VEML7700_GAIN_1);              // Options: VEML7700_GAIN_1/8, VEML7700_GAIN_1/4, VEML7700_GAIN_1, VEML7700_GAIN_2
  veml.setIntegrationTime(VEML7700_IT_100MS); // Options: 25MS, 50MS, 100MS, 200MS, 400MS, 800MS

  //#################################################ENS160#################################################

  bool ok = ens160.begin();
  Serial.println(ens160.available() ? "done." : "failed!");
  if (ens160.available()) {
    Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");
  }

  //#################################################NEOPIXEL#################################################

  FastLED.clear();
  
  FastLED.addLeds<NEOPIXEL, NEO_PIN>(leds, NUM_LEDS); 

  // quick neo test on boot
  leds[0] = CRGB::Red; FastLED.show(); delay(500);
  leds[0] = CRGB::Blue; FastLED.show(); delay(500);
  leds[0] = CRGB::Green; FastLED.show(); delay(500);

  FastLED.clear();
  
}

void loop() {

  //#################################################SHT41#################################################

  uint8_t shtdata[6];

  // SHT41 I2C Communication
  Wire.beginTransmission(SHT41_ADDR); // Start coms with device
  Wire.write(HIGHR_ADDR);             // send reg addr we want
  Wire.endTransmission();             // this is what actually sends the command

  delay(10);                          // wait for the sensor to measure (max 8.3ms on high repeatability mode)

  Wire.requestFrom(SHT41_ADDR, 6);    // receive 6 bytes
  if (Wire.available() < 6) return;   // bail this loop iteration if the bytes aren't there yet

  for (uint8_t i = 0; i < 6; i++) shtdata[i] = Wire.read();

  // shift the high byte up and OR in the low byte, for both temp and humidity
  uint16_t t_fullbyte  = ((uint16_t)shtdata[0] << 8) | shtdata[1];
  uint16_t rh_fullbyte = ((uint16_t)shtdata[3] << 8) | shtdata[4];

  convertSHT41(rh_fullbyte, t_fullbyte);

  //#################################################VEML7700#################################################

  int luxval = veml.readLux();
  Serial.print("luxval: ");
  Serial.println(luxval);

  //#################################################ENS160#################################################

  if (ens160.available()) {

    ens160.measureRaw(true);

    Serial.print("R HP0: ");Serial.print(ens160.getHP0());Serial.print("Ohm\t");
    Serial.print("R HP1: ");Serial.print(ens160.getHP1());Serial.print("Ohm\t");
    Serial.print("R HP2: ");Serial.print(ens160.getHP2());Serial.print("Ohm\t");
    Serial.print("R HP3: ");Serial.print(ens160.getHP3());Serial.println("Ohm");
  }

  delay(500);
}


