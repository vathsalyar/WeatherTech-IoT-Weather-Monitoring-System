#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <RTClib.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD at address 0x27
RTC_DS3231 rtc;

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define UV_PIN A1
int buz = 8;
int led = 9;

const int aqsensor = A0;  // MQ135 output pin
int threshold = 250;

// Display durations
const int tempHumidDisplayTime = 3000;
const int airQualityDisplayTime = 3000;

void setup() {
  Serial.begin(9600);
  dht.begin();
  rtc.begin();
  lcd.init();
  lcd.backlight();
  pinMode(buz, OUTPUT);
  pinMode(led, OUTPUT);
}

void loop() {
  // Read sensors
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int ppm = analogRead(aqsensor);
  DateTime now = rtc.now();

  // Store data in EEPROM
  int address = 0;
  EEPROM.put(address, humidity); address += sizeof(humidity);
  EEPROM.put(address, temperature); address += sizeof(temperature);
  EEPROM.put(address, ppm); address += sizeof(ppm);
  float uvIndex = getUVIndex();
  EEPROM.put(address, uvIndex);

  // Display Temperature & Humidity
  lcd.setCursor(0, 0);
  lcd.print("Humidity= ");
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Temp=");
  lcd.print(temperature);
  lcd.print(" C");
  delay(tempHumidDisplayTime);

  // Display Air Quality
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Air Quality:");
  lcd.setCursor(12, 0);
  lcd.print(ppm);
  if (ppm > threshold) {
    lcd.setCursor(0, 1);
    lcd.print("AQ Level HIGH");
    tone(buz, 1000, 200);
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
    noTone(buz);
    lcd.setCursor(0, 1);
    lcd.print("AQ Level Good");
  }
  delay(airQualityDisplayTime);

  // Display UV Index
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UV Index:");
  lcd.print(uvIndex);
  if (uvIndex > 3) {
    lcd.setCursor(0, 1);
    lcd.print("UV Level HIGH");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("UV Level Good");
  }
  delay(3000);

  // Display Date & Time
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  lcd.print(now.year());
  lcd.print("/");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.day());
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute());
  lcd.print(":");
  lcd.print(now.second());
  delay(3000);

  // Print sensor data to Serial
  Serial.print("Humidity:");
  Serial.println(humidity);
  Serial.print("Temperature:");
  Serial.println(temperature);
  Serial.print("Air Quality (ppm):");
  Serial.println(ppm);
  Serial.print("UV Index:");
  Serial.println(uvIndex);
}

// Function to calculate UV index
float getUVIndex() {
  int sensorValue = analogRead(UV_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);
  return voltage * 3.33; // calibration factor
}
