#define BLYNK_TEMPLATE_ID "TMPL6Txg71nsp"
#define BLYNK_TEMPLATE_NAME "IoT Energy Meter"
#define BLYNK_AUTH_TOKEN "UpftZmtlj36wah_Ms7-MmRkk35vyEA0V"
#define BLYNK_PRINT Serial

#include "EmonLib.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Calibration constants
const float vCalibration = 41.5;
const float currCalibration = 0.15;

// WiFi credentials
const char ssid[] = "Maisha";
const char pass[] = "fmaisha55";

// EnergyMonitor instance
EnergyMonitor emon;

// Timer
BlynkTimer timer;

// Energy variables
float kWh = 0.0;
unsigned long lastMillis = 0;

// EEPROM addresses
const int addrKWh = 0;

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
}

void setup() {
  Serial.begin(115200);

  // EEPROM initialization
  EEPROM.begin(512);
  kWh = EEPROM.readFloat(addrKWh);
  if (isnan(kWh)) kWh = 0;

  // Setup EmonLib
  emon.voltage(35, vCalibration, 1.7);
  emon.current(34, currCalibration);

  // LCD setup
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Energy Meter");
  delay(1000);

  // WiFi & Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lastMillis = millis();
  timer.setInterval(3000L, sendEnergyDataToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}

void sendEnergyDataToBlynk() {
  emon.calcVI(20, 2000);

  float volt = emon.Vrms;
  float current = emon.Irms;
  float power = emon.realPower;

  unsigned long now = millis();
  float elapsed = (now - lastMillis) / 3600000.0;
  lastMillis = now;

  kWh += (power * elapsed) / 1000.0;

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(volt, 1);
  lcd.print(" I:");
  lcd.print(current, 2);

  lcd.setCursor(0, 1);
  lcd.print("P:");
  lcd.print(power, 1);
  lcd.print(" kWh:");
  lcd.print(kWh, 4);

  // Serial output
  Serial.printf("Vrms: %.2fV, Irms: %.4fA, Power: %.4fW, kWh: %.5f\n",
                volt, current, power, kWh);

  // Save to EEPROM
  EEPROM.writeFloat(addrKWh, kWh);
  EEPROM.commit();

  // Send to Blynk
  Blynk.virtualWrite(V0, volt);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, kWh);
}
