#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <RTClib.h>

// ========== LoRa pin definition ===========
#define LoRa_SCK 18
#define LoRa_MISO 19
#define LoRa_MOSI 23
#define LoRa_SS 5
#define LoRa_RST 13
#define LoRa_DIO0 26

// ========== LED / Relay Control ==========
// ลองเปลี่ยนเป็น 2 ถ้าอยากเทสกับไฟสีฟ้าบนบอร์ด (Built-in LED)
#define AIR_PURIFIER_LED 4  

// ============== LoRa Config =================
const long LoRa_FREQ = 433E6;
const uint8_t SYNC_WORD = 0x12;

// ================ ID ==================
const char* GROUP_ID = "G9";
char deviceId = 'R';

// ================ WiFi =================
const char* ssid = "Xiaomi 14T Pro";
const char* password = "stangpat1234";

// ================ Firebase =================
const char* FIREBASE_HOST = "https://weather-app-a8630-default-rtdb.asia-southeast1.firebasedatabase.app";

// ================ RTC =================
RTC_DS3231 rtc;

// ================ Variables =================
const unsigned long ONE_WEEK = 7UL * 24UL * 60UL * 60UL;
unsigned long lastDeleteTs = 0;
bool airPurifierActive = false; 

// --------- Control Air Purifier Function (Debug Version) ----------
void controlAirPurifier(float dust) {
  // เงื่อนไข: ถ้าฝุ่นมากกว่า 50 ให้เปิด
  bool shouldActivate = (dust >= 26.0);
  
  Serial.printf("DEBUG LED: Dust=%.2f -> Need ON? %s\n", dust, shouldActivate ? "YES" : "NO");

  if (shouldActivate) {
    digitalWrite(AIR_PURIFIER_LED, HIGH); // สั่งเปิดไฟ (High Voltage)
    airPurifierActive = true;
    Serial.println("Command: LED ON (HIGH)");
  }
  else {
    digitalWrite(AIR_PURIFIER_LED, LOW); // สั่งปิดไฟ (Low Voltage)
    airPurifierActive = false;
    Serial.println("Command: LED OFF (LOW)");
  }
}

// --------- Send to Firebase ----------
void sendToFirebase(float t, float h, float p, float d, float b) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected! Cannot send to Firebase.");
    return;
  }
  
  DateTime now = rtc.now();
  unsigned long ts = now.unixtime();

  char dt[20];
  sprintf(dt, "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());

  String url = String(FIREBASE_HOST) + "/air_quality/G9/N1/" + String(ts) + ".json";

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"ts\":" + String(ts) + ",";
  json += "\"datetime\":\"" + String(dt) + "\",";
  json += "\"temp\":" + String(t, 1) + ",";
  json += "\"humi\":" + String(h, 1) + ",";
  json += "\"pres\":" + String(p, 1) + ",";
  json += "\"dust\":" + String(d, 1) + ",";
  json += "\"batt\":" + String(b, 2) + ",";
  json += "\"air_purifier\":\"" + String(airPurifierActive ? "ON" : "OFF") + "\"";
  json += "}";

  int code = http.PUT(json);
  Serial.print("Firebase status: ");
  Serial.println(code);
  http.end();
}

// ================== Delete Old Data ===================
void deleteOldData() {
  if (WiFi.status() != WL_CONNECTED) return;
  unsigned long nowTs = rtc.now().unixtime();
  unsigned long cutoffTs = nowTs - ONE_WEEK;
  String url = String(FIREBASE_HOST) + "/air_quality/" + GROUP_ID + "/" + deviceId + ".json?orderBy=\"ts\"&endAt=" + String(cutoffTs);
  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  if (code == 200) {
      // Logic การลบ (ย่อไว้)
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ===== 1. LED Config (ตั้งค่าขาไว้ก่อน) =====
  pinMode(AIR_PURIFIER_LED, OUTPUT);
  digitalWrite(AIR_PURIFIER_LED, LOW); // ปิดไว้ก่อน
  Serial.println("LED GPIO Configured");

  // ===== 2. RTC Setup =====
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // ===== 3. LoRa Setup =====
  SPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_SS);
  LoRa.setSPI(SPI);
  LoRa.setPins(LoRa_SS, LoRa_RST, LoRa_DIO0);

  if (!LoRa.begin(LoRa_FREQ)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();
  LoRa.setSyncWord(SYNC_WORD);
  Serial.println("LoRa RX Ready");

  // ===== 4. WiFi Setup (จุดที่มักจะค้าง) =====
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    // ให้ไฟกระพริบสั้นๆ ระหว่างรอ WiFi เพื่อบอกว่ายังไม่ตาย
    digitalWrite(AIR_PURIFIER_LED, !digitalRead(AIR_PURIFIER_LED));
    delay(500);
    Serial.print(".");
  }
  digitalWrite(AIR_PURIFIER_LED, LOW); // ปิดเมื่อต่อติด
  Serial.println("\nWiFi connected");

  // ===== 5. Final Test (กระพริบ 3 ครั้งเพื่อบอกว่าพร้อมทำงาน) =====
  Serial.println("System Ready! Blinking LED...");
  for(int i=0; i<3; i++) {
    digitalWrite(AIR_PURIFIER_LED, HIGH);
    delay(200);
    digitalWrite(AIR_PURIFIER_LED, LOW);
    delay(200);
  }
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String msg = "";
    while (LoRa.available()) {
      msg += (char)LoRa.read();
    }
    msg.trim();

    if (!msg.startsWith("G9")) return;

    Serial.println("-----------------------------");
    Serial.println("RECV: " + msg);

    float t, h, p, d, b;
    int n = sscanf(msg.c_str(), "G9,T=%f,H=%f,P=%f,D=%f,B=%f", &t, &h, &p, &d, &b);

    if (n == 5) {
      // 1. ควบคุม LED
      controlAirPurifier(d);

      // 2. ส่งข้อมูล
      sendToFirebase(t, h, p, d, b);
    } else {
      Serial.println("Parse Error: ข้อมูลไม่ครบ 5 ค่า");
    }
  }
}