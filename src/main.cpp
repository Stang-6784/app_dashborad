#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

// I2C address for BME280 (0x76 or 0x77)
#define BME_ADDRESS 0x76

Adafruit_BME280 bme; 

// กำหนด Baud Rate เป็น 9600 เพื่อให้ตรงกับ Flask Server
#define BAUD_RATE 9600 

void setup() {
    Wire.begin();
    Serial.begin(BAUD_RATE);
    Serial.println("Initialzing BME280...");

    // ตรวจสอบการเชื่อมต่อ BME280
    while (!bme.begin(BME_ADDRESS)) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nBME280 initialization successful!");
    // กำหนดค่า Sampling (optional)
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_OFF);
}

// ฟังก์ชันนี้ถูกลบออก เพราะเราจะอ่านและส่งค่าทั้งหมดใน loop()
/* float tempRead() { ... } */

void loop() {
    // อ่านค่า BME280
    bme.takeForcedMeasurement(); // สั่งให้เซนเซอร์อ่านค่า

    float temperature = bme.readTemperature(); // อุณหภูมิ (°C)
    float humidity = bme.readHumidity();       // ความชื้น (%)
    float pressure = bme.readPressure() / 100.0F; // ความกดอากาศ (hPa)

    // ตรวจสอบว่าค่าที่อ่านได้มีความสมเหตุสมผลหรือไม่ (ป้องกันค่าที่ไม่ถูกต้องตอนเริ่มต้น)
    if (isnan(temperature) || isnan(humidity) || isnan(pressure)) {
        Serial.println("Error reading sensor!");
    } else {
        // *** รูปแบบการส่งข้อมูลสำคัญมาก: CSV (Comma-Separated Values) ***
        // ส่ง: อุณหภูมิ,ความชื้น,ความกดอากาศ (แทน humidity,dust ในโค้ด Python เดิม)
        // ใช้ 2 ตำแหน่งทศนิยมเพื่อความแม่นยำและง่ายต่อการ Parse
        Serial.print(String(temperature, 2));
        Serial.print(",");
        Serial.print(String(humidity, 2));
        Serial.print(",");
        Serial.println(String(pressure, 2));

        // Note: สามารถเปิดบรรทัดนี้เพื่อดูการส่งค่าใน Serial Monitor
        // Serial.print("Sent: ");
        // Serial.print(String(temperature, 2)); Serial.print(",");
        // Serial.print(String(humidity, 2)); Serial.print(",");
        // Serial.println(String(pressure, 2));
    }

    // ส่งข้อมูลทุกๆ 2 วินาที
    // delay(2000);
}
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