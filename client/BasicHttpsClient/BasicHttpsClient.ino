/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
//const uint8_t fingerprint[20] = { 0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3 };

ESP8266WiFiMulti WiFiMulti;
int filter = D5;
int light = D6;
int co2 = D7;
void setup() {

  pinMode(filter, OUTPUT);
  digitalWrite(filter, HIGH);
  pinMode(light, OUTPUT);
  digitalWrite(light, HIGH);
  pinMode(co2, OUTPUT);
  digitalWrite(co2, HIGH);

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("TEA@2.4GHz", "insta11wifi");
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

//    client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
     client->setInsecure();

    HTTPClient https;
    StaticJsonDocument<100> doc;
    if (https.begin(*client, "https://192.168.1.7:3000/sync")) {  // HTTPS
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String message = https.getString();
//          Serial.println();
          DeserializationError err = deserializeJson(doc, message);
    if (err)
    {
    }
    else
    {
      Serial.println(message);
      boolean filterStatus = doc["filter"];
      boolean lightStatus = doc["light"];
      boolean co2Status = doc["co2"];
      if (filterStatus)
      {
        digitalWrite(filter, LOW);
      }
      else
      {
        digitalWrite(filter, HIGH);
      }
      if (lightStatus)
      {
        digitalWrite(light, LOW);
      }
      else
      {
        digitalWrite(light, HIGH);
      }
      if (co2Status)
      {
        digitalWrite(co2, LOW);
      }
      else
      {
        digitalWrite(co2, HIGH);
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
