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
#define LoRa_RST 4
#define LoRa_DIO0 26

// ============== LoRa Config =================
const long LoRa_FREQ = 433E6;
const uint8_t SYNC_WORD = 0x12;

// ================ ID ==================
const char* GROUP_ID = "G9";
char deviceId = 'R';

// ================ WiFi =================
const char* ssid = "EMB5326";
const char* password = "cdti12345";

// ================ Firebase =================
const char* FIREBASE_HOST =
"https://weather-app-a8630-default-rtdb.asia-southeast1.firebasedatabase.app";

// ================ RTC =================
RTC_DS3231 rtc;

// ================ Delete Data =================
const unsigned long ONE_WEEK = 7UL * 24UL * 60UL * 60UL;
unsigned long lastDeleteTs = 0;

// --------- Get Unix Timestamp ----------
unsigned long getTimestamp() {
  DateTime now = rtc.now();
  return now.unixtime();
}

// --------- Send to Firebase ----------
void sendToFirebase(float t, float h, float p, float d, float b) {
  if (WiFi.status() != WL_CONNECTED) return;
  DateTime now = rtc.now();

  // Unix timestamp (ใช้เป็น key)
  unsigned long ts = now.unixtime();

  // String วันที่เวลา (อ่านง่าย)
  char dt[20];
  sprintf(dt, "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());

  String url =
  String(FIREBASE_HOST) +
  "/air_quality/G9/N1/" +
  String(ts) +
  ".json";

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"ts\":" + String(ts) + ",";
  json += "\"datetime\":\"" + String(dt) + "\",";
  json += "\"temp\":" + String(t,1) + ",";
  json += "\"humi\":" + String(h,1) + ",";
  json += "\"pres\":" + String(p,1) + ",";
  json += "\"dust\":" + String(d,1) + ",";
  json += "\"batt\":" + String(b,2);
  json += "}";
  

  int code = http.PUT(json);
  Serial.print("Firebase HTTP code = ");
  Serial.println(code);

if (code > 0) {
  String payload = http.getString();
  Serial.println(payload);
}

  http.end();
}

// ================== Delete Old Data ===================
void deleteOldData() {
  if (WiFi.status() != WL_CONNECTED) return;

  unsigned long nowTs = rtc.now().unixtime();
  unsigned long cutoffTs = nowTs - ONE_WEEK;

  String url = String(FIREBASE_HOST) +
    "/air_quality/" + GROUP_ID + "/" + deviceId + ".json"
    "?orderBy=\"ts\"&endAt=" + String(cutoffTs);

  HTTPClient http;
  http.begin(url);

  int code = http.GET();
  if (code == 200) {
    String payload = http.getString();

    if (payload != "null") {
      HTTPClient del;
      String delUrl = String(FIREBASE_HOST) +
        "/air_quality/" + GROUP_ID + "/" + deviceId + ".json"
        "?orderBy=\"ts\"&endAt=" + String(cutoffTs);

      del.begin(delUrl);
      del.sendRequest("DELETE");
      del.end();

      Serial.println("Old data deleted (older than 1 week)");
    }
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ===== RTC setup =====
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC not found");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // ===== LoRa setup =====
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

  // ===== WiFi =====
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {

  // รับข้อมูล LoRa 
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return;

  String msg = LoRa.readString();
  msg.trim();

  if (!msg.startsWith("G9")) return;

  Serial.println("RECV = " + msg);

  // ส่งขึ้น Firebase
  float t,h,p,d,b;
  if (sscanf(msg.c_str(),
      "G9,T=%f,H=%f,P=%f,D=%f,B=%f",
      &t,&h,&p,&d,&b) == 5) {

    sendToFirebase(t,h,p,d,b);
  }
}