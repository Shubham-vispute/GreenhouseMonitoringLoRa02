#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

#define SOIL_PIN 34
#define LDR_PIN 35
#define MQ135_PIN 32

#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 5
#define RST 14
#define DIO0 26

#define RELAY_PIN 2

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Wire.begin(21, 22);
  if (!tsl.begin()) {
    Serial.println("TSL2561 not found!");
    while (1);
  }
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);

  SPI.begin(SCK, MISO, MOSI, CS);
  LoRa.setPins(CS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }

  Serial.println("LoRa Transmitter Ready");
}

void loop() {
  // 1. Always check for incoming commands
  if (LoRa.parsePacket()) {
    String command = "";
    while (LoRa.available()) {
      command += (char)LoRa.read();
    }
    Serial.println("Received command: " + command);

    if (command.startsWith("relay:")) {
      int state = command.substring(6).toInt();
      digitalWrite(RELAY_PIN, state == 1 ? HIGH : LOW);
      Serial.println(state == 1 ? "Relay ON" : "Relay OFF");
    }
  }

  // 2. Send data every 10 seconds
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int soil = analogRead(SOIL_PIN);
    sensors_event_t event;
    tsl.getEvent(&event);
    float lux = event.light ? event.light : 0;
    int air = analogRead(MQ135_PIN);

    String data = "T:" + String(temp) + ",H:" + String(hum) + ",S:" + String(soil) +
                  ",L:" + String(lux) + ",A:" + String(air);

    LoRa.beginPacket();
    LoRa.print(data);
    LoRa.endPacket();
    Serial.println("Sent: " + data);
  }
}
