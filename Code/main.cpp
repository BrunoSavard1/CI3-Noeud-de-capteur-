// Libraries
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_VEML7700.h"
#include "ScioSense_ENS160.h"  // ENS160 library
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// Defines
#define SHT41_ADDR 0x44  // I2C address
#define HIGHR_ADDR 0xFD  // high repeatability measurement command
#define NEO_PIN    3

// See the following for generating UUIDs:
// https://uuidgenerator.net
#define SERVICE_UUID        "68128d14-d63a-4cb7-b6db-c4414b736391"
#define CHARACTERISTIC_UUID "26359e86-abd4-4d60-9436-269e38c77c2c"

// I2C pins
#define I2C_SDA 6
#define I2C_SCL 5

// Variables and objects
Adafruit_VEML7700 veml = Adafruit_VEML7700();
ScioSense_ENS160  ens160(ENS160_I2CADDR_0);  // 0x52 (use ENS160_I2CADDR_1 for 0x53)

// to check sensor availability
bool vemlOK = false;
bool ensOK  = false;

// for BLE
BLECharacteristic *pCharacteristic = nullptr;
volatile bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    deviceConnected = true;
    Serial.println("BLE client connected");
    neopixelWrite(NEO_PIN, 0, 0, 5);
  }
  void onDisconnect(BLEServer *pServer) override {
    deviceConnected = false;
    Serial.println("BLE client disconnected, advertising again");
    BLEDevice::startAdvertising();  // allow reconnecting
    neopixelWrite(NEO_PIN, 0, 5, 0);
  }
};

const char* aqiLabel(uint8_t aqi) {
  switch (aqi) {
    case 1: return "Excellent";
    case 2: return "Good";
    case 3: return "Moderate";
    case 4: return "Poor";
    case 5: return "Unhealthy";
    default: return "Unknown";
  }
}

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 5000) delay(10);  // wait up to 5 s for the monitor
  delay(500);
  Serial.println("Booting...");

  Wire.begin(I2C_SDA, I2C_SCL);  // I2C init with explicit pins

  // ################ I2C SCAN (shows what is really on the bus) ################
  Serial.println("Scanning I2C bus...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  I2C device found at 0x%02X\n", addr);
    }
  }
  Serial.println("Scan done.");

  // ################################ VEML7700 ################################
  vemlOK = veml.begin();
  if (!vemlOK) {
    Serial.println("VEML7700 not found!");
  } else {
    Serial.println("VEML7700 ok.");
    veml.setGain(VEML7700_GAIN_1);               // Options: GAIN_1_8, GAIN_1_4, GAIN_1, GAIN_2
    veml.setIntegrationTime(VEML7700_IT_100MS);  // Options: 25MS, 50MS, 100MS, 200MS, 400MS, 800MS
  }

  // ################################# ENS160 #################################
  ensOK = ens160.begin() && ens160.available();
  if (!ensOK) {
    Serial.println("ENS160 failed!");
  } else {
    Serial.println("ENS160 ok.");
    Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "ENS160 mode set." : "ENS160 mode failed!");
  }

  // ################################### BLE ###################################
  BLEDevice::init("ESP32-C3-BLE-SEVVV");
  BLEDevice::setMTU(247);  // allow long notifications (phone must agree to it)

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // register the callbacks

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // NOTE: no "BLECharacteristic *" here, so the GLOBAL pointer gets assigned
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_WRITE |
      BLECharacteristic::PROPERTY_NOTIFY);  // notify is required for live updates
  pCharacteristic->addDescriptor(new BLE2902());  // lets the phone enable notifications

  pCharacteristic->setValue("Hello World from ESP32-C3");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // helps with iPhone connection issues
  pAdvertising->setMaxPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE advertised. Connect with your phone or BLE scanner.");

  Serial.println("Setup done.");
  Serial.flush();

  neopixelWrite(NEO_PIN, 0, 5, 0);
}

void loop() {
  // Declared here so they are visible to the BLE payload at the bottom
  float SHT_Temp = NAN;
  float SHT_RH   = NAN;
  float luxval   = NAN;
  uint8_t  aqi  = 0;   // 1-5 (UBA index)
  uint16_t tvoc = 0;   // ppb
  uint16_t eco2 = 0;   // ppm

  // ################################## SHT41 ##################################
  uint8_t shtdata[6];

  Wire.beginTransmission(SHT41_ADDR);    // start comms with device
  Wire.write(HIGHR_ADDR);                // send the command we want
  uint8_t err = Wire.endTransmission();  // this actually sends it

  if (err != 0) {
    Serial.printf("SHT41 not responding (I2C error %u)\n", err);
  } else {
    delay(10);  // wait for measurement (max 8.3 ms in high repeatability mode)

    Wire.requestFrom((uint8_t)SHT41_ADDR, (uint8_t)6);  // receive 6 bytes
    if (Wire.available() >= 6) {
      for (uint8_t i = 0; i < 6; i++) shtdata[i] = Wire.read();

      // shift the high byte up and OR in the low byte (temp and humidity)
      uint16_t t_fullbyte  = ((uint16_t)shtdata[0] << 8) | shtdata[1];
      uint16_t rh_fullbyte = ((uint16_t)shtdata[3] << 8) | shtdata[4];

      SHT_Temp = -45.0f + 175.0f * ((float)t_fullbyte  / 65535.0f);  // in °C
      SHT_RH   = -6.0f  + 125.0f * ((float)rh_fullbyte / 65535.0f);  // in %

      Serial.print("SHT_RH: ");
      Serial.println(SHT_RH);

      Serial.print("SHT_Temp: ");
      Serial.println(SHT_Temp);
    } else {
      Serial.println("SHT41: not enough bytes received");
    }
  }

  // ################################ VEML7700 ################################
  if (vemlOK) {
    luxval = veml.readLux();
    Serial.print("luxval: ");
    Serial.println(luxval);
  }

  // ################################# ENS160 #################################
  if (ensOK) {
    // Feed the real temp/humidity so the readings get compensated (only if SHT41 gave valid data)
    if (!isnan(SHT_Temp) && !isnan(SHT_RH)) {
      ens160.set_envdata(SHT_Temp, SHT_RH);
    }

    if (ens160.measure(true)) {  // true = wait for new data
      aqi  = ens160.getAQI();
      tvoc = ens160.getTVOC();
      eco2 = ens160.geteCO2();

      Serial.printf("AQI: %u (%s)\tTVOC: %u ppb\teCO2: %u ppm\n",
                    aqi, aqiLabel(aqi), tvoc, eco2);
    } else {
      Serial.println("ENS160: measure failed");
    }
  }

  // ################################### BLE ###################################
  if (deviceConnected && pCharacteristic) {
    char payload[160];
    snprintf(payload, sizeof(payload),
            "Temp:%.1fC Humidity:%.1f%% Lux:%.0f AQI:%u TVOC:%u eCO2:%u",
            SHT_Temp, SHT_RH, luxval, aqi, tvoc, eco2);
    pCharacteristic->setValue((uint8_t *)payload, strlen(payload));
    pCharacteristic->notify();  // push to the phone
  }

  Serial.println();
  delay(1000);
}