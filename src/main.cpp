#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

#define SDA_PIN 21
#define SCL_PIN 22

// 1. สร้าง Object สำหรับเซนเซอร์แต่ละตัว
Adafruit_BME280 bmeA; // สำหรับ Sensor A
Adafruit_BME280 bmeB; // สำหรับ Sensor B

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("--- BME280 Dual Sensor Setup ---");

  // เริ่มต้น I2C Bus บน ESP32
  Wire.begin(SDA_PIN, SCL_PIN); 

  // 2. เริ่มต้น Sensor A ด้วย Address 0x76
  if (!bmeA.begin(0x76, &Wire)) {
    Serial.println("❌ ไม่พบ BME280 Sensor A ที่ 0x76");
    // คุณอาจเปลี่ยนไปลอง 0x77 ก็ได้ แต่เราสมมติว่ามีการแยกแอดเดรสไว้แล้ว
  } else {
    Serial.println("✅ BME280 Sensor A (0x76) พร้อมใช้งาน");
  }

  // 3. เริ่มต้น Sensor B ด้วย Address 0x77
  if (!bmeB.begin(0x77, &Wire)) {
    Serial.println("❌ ไม่พบ BME280 Sensor B ที่ 0x77");
  } else {
    Serial.println("✅ BME280 Sensor B (0x77) พร้อมใช้งาน");
  }

  Serial.println("--------------------------------");
}

void loop() {
  // อ่านและแสดงค่าจาก Sensor A (0x76)
  Serial.println("--- Sensor A (Outside) ---");
  Serial.print("Temp A: ");
  Serial.print(bmeA.readTemperature());
  Serial.println(" °C");

  Serial.print("Humidity A: ");
  Serial.print(bmeA.readHumidity());
  Serial.println(" %");

  Serial.print("Pressure A: ");
  Serial.print(bmeA.readPressure() / 100.0F); // แปลง Pa เป็น hPa
  Serial.println(" hPa");

  // อ่านและแสดงค่าจาก Sensor B (0x77)
  Serial.println("--- Sensor B (Inside) ---");
  Serial.print("Temp B: ");
  Serial.print(bmeB.readTemperature());
  Serial.println(" °C");

  Serial.print("Humidity B: ");
  Serial.print(bmeB.readHumidity());
  Serial.println(" %");

  Serial.print("Pressure B: ");
  Serial.print(bmeB.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.println("--------------------------------");
  delay(5000); // หน่วงเวลา 5 วินาที
}
// #define BME280PinScl 22
// #define BME280PinSda 21
// Adafruit_BME280 bme; 

// int sensorValueSDA;
// int sensorValueSCL;

// void setup() {
//   Serial.begin(9600);
//   Serial.println("BME280 test");
//   Wire.begin(BME280PinSda, BME280PinScl);
//   if (!bme.begin(0x76)) {
//     Serial.println("ลอง address 0x77");
//     if (!bme.begin(0x77)) {
//       Serial.println("ไม่พบ BME280 ตรวจสอบสาย/ไฟ");
//       while (1);
//     }
//     Serial.println("BME280 พร้อมใช้งาน!");
//   }
// }

// void loop() {
//   float temp = bme.readTemperature();
//   float humidity = bme.readHumidity();
//   // แปลงความดันเป็น hPa
//   float pressure = bme.readPressure() / 100.0F;

//   // *** รูปแบบ CSV: Temp,Humidity,Pressure ***
//   // โค้ด Python จะอ่านและแยกค่าเหล่านี้ด้วยเครื่องหมาย comma (,)
//   Serial.print(temp);
//   Serial.print(",");
//   Serial.print(humidity);
//   Serial.print(",");
  
//   // ใช้ Serial.println() เพื่อขึ้นบรรทัดใหม่ ทำให้ Python อ่านเป็น 1 ชุดข้อมูลได้ง่าย
//   Serial.println(pressure);
//   delay(2000);
// }