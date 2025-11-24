#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

#define BME280PinScl 22
#define BME280PinSda 21
Adafruit_BME280 bme; // I2C

int sensorValueSDA;
int sensorValueSCL;

void setup() {
  Serial.begin(9600);
  Serial.println("BME280 test");
  Wire.begin(BME280PinSda, BME280PinScl);
  if (!bme.begin(0x76)) {
    Serial.println("ลอง address 0x77");
    if (!bme.begin(0x77)) {
      Serial.println("ไม่พบ BME280 ตรวจสอบสาย/ไฟ");
      while (1);
    }
    Serial.println("BME280 พร้อมใช้งาน!");
  }
}

void loop() {
 Serial.print("Temp: ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.print("Pressure: ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.println("------------------");
  delay(2000);
}