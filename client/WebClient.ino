#include <ArduinoJson.h>

/*
 WiFiEsp example: WebClient

 This sketch connects to google website using an ESP8266 module to
 perform a simple web search.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp-example-client.html
*/

#include "WiFiEsp.h"
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = "TEA@2.4GHz";  // your network SSID (name)
char pass[] = "insta11wifi"; // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status

char server[] = "192.168.1.14";

// Initialize the Ethernet client object
WiFiEspClient client;
int filter = 8;
int light = 9;
int pump = 10;
void setup()
{
  pinMode(filter, OUTPUT);
  digitalWrite(filter, HIGH);
  pinMode(light, OUTPUT);
  digitalWrite(light, HIGH);
  pinMode(pump, OUTPUT);
  digitalWrite(pump, HIGH);

  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true)
      ;
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  printWifiStatus();

  Serial.println();
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial
  if (client.connect(server, 3030))
  {
    Serial.println("Connected to server");
  }
}

void loop()
{
  // if there are incoming bytes available
  // from the server, read them and print them
  StaticJsonDocument<100> doc;
  while (client.connected())
  {
    String message;
    while (client.available())
    {
      char c = client.read();
      message += c;
    }
    DeserializationError err = deserializeJson(doc, message);
    if (err)
    {
    }
    else
    {
      Serial.println(message);
      boolean filterStatus = doc["filter"];
      boolean lightStatus = doc["light"];
      boolean pumpStatus = doc["pump"];
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
      if (pumpStatus)
      {
        digitalWrite(pump, LOW);
      }
      else
      {
        digitalWrite(pump, HIGH);
      }
    }
  }

  // if the server's disconnected, stop the client
  while (!client.connected())
  {
    Serial.print("Attempting to reconnect...");
    client.connect(server, 3030);
    delay(1000);
    // do nothing forevermore
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
