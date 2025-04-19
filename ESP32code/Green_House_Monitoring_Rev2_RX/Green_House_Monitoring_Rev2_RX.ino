
#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#define SS 15    // D8
#define RST 16   // D0
#define DIO0 5   // D1

#define WIFI_SSID "motorola edge 50 pro_2882"
#define WIFI_PASSWORD "7276xxxxxxxxx"

#define FIREBASE_HOST "your-project.firebaseio.com"
#define FIREBASE_AUTH "key_firebase"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

float temp = 0.0, hum = 0.0;
int soil = 0, light = 0, air = 0;
bool dataReady = false;

unsigned long lastFirebaseUpdate = 0;
const unsigned long firebaseInterval = 10000; // 10 seconds

float extractValue(String data, String label) {
  int start = data.indexOf(label + ":") + 2;
  int end = data.indexOf(',', start);
  if (end == -1) end = data.length();
  return data.substring(start, end).toFloat();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  SPI.begin();
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed");
    while (1);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  Serial.println("ESP8266 Receiver Ready");
}

void loop() {
  // 1. LoRa receive
  if (LoRa.parsePacket()) {
    String payload = LoRa.readString();
    Serial.println("Received: " + payload);

    temp = extractValue(payload, "T");
    hum = extractValue(payload, "H");
    soil = extractValue(payload, "S");
    light = extractValue(payload, "L");
    air = extractValue(payload, "A");

    dataReady = true;
  }

  // 2. Firebase update and relay send
  if (millis() - lastFirebaseUpdate >= firebaseInterval && dataReady) {
    lastFirebaseUpdate = millis();

    Firebase.setFloat(fbdo, "/Greenhouse/Temperature", temp);
    Firebase.setFloat(fbdo, "/Greenhouse/Humidity", hum);
    Firebase.setInt(fbdo, "/Greenhouse/SoilMoisture", soil);
    Firebase.setInt(fbdo, "/Greenhouse/Light", light);
    Firebase.setInt(fbdo, "/Greenhouse/AirQuality", air);

    if (Firebase.getInt(fbdo, "/relay")) {
      int relayCommand = fbdo.intData();
      Serial.println("Sending relay: " + String(relayCommand));
      LoRa.beginPacket();
      LoRa.print("relay:" + String(relayCommand));
      LoRa.endPacket();
    }

    dataReady = false;
  }
}
