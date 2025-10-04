#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

ESP8266WiFiMulti WiFiMulti;

int filter = D5;
int light = D6;
int co2 = D7;

// Track previous light status
bool prevLightStatus = false;

void setup() {
  pinMode(filter, OUTPUT);
  digitalWrite(filter, HIGH);

  pinMode(light, OUTPUT);
  digitalWrite(light, HIGH);

  pinMode(co2, OUTPUT);
  digitalWrite(co2, HIGH);

  Serial.begin(115200);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("TEA@2.4GHz", "insta11wifi");
}

void handleLightMode(const String &mode) {
  if (mode == "wb") {
    digitalWrite(light, LOW); // turn on
  } 
  else if (mode == "rgb") {
    digitalWrite(light, LOW);
    delay(200); // short delay to prevent relay jitter
    digitalWrite(light, HIGH);
    delay(200);
    digitalWrite(light, LOW);
  } 
  else if (mode == "wbrgb") {
    digitalWrite(light, LOW);
    delay(200);
    digitalWrite(light, HIGH);
    delay(200);
    digitalWrite(light, LOW);
    delay(200);
    digitalWrite(light, HIGH);
    delay(200);
    digitalWrite(light, LOW);
  }
}

void loop() {
  if (WiFiMulti.run() == WL_CONNECTED) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    HTTPClient https;
    StaticJsonDocument<200> doc;

    if (https.begin(*client, "https://192.168.1.28:3000/sync")) {
      int httpCode = https.GET();

      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String message = https.getString();
          DeserializationError err = deserializeJson(doc, message);

          if (!err) {
            Serial.println(message);

            // Read statuses from server
            boolean filterStatus = doc["filter"];
            boolean lightStatus = doc["light"];
            boolean co2Status = doc["co2"];
            String mode = doc["mode"] | "wb"; // default mode

            // Filter
            digitalWrite(filter, filterStatus ? LOW : HIGH);

            // CO2
            digitalWrite(co2, co2Status ? LOW : HIGH);

            // Light handling
            if (lightStatus != prevLightStatus) {
              prevLightStatus = lightStatus; // update previous
              if (lightStatus) {
                handleLightMode(mode);
              } else {
                digitalWrite(light, HIGH); // turn off
              }
            }

          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  delay(1000);
}
