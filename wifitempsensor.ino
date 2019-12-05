#include "headers/credentials.h"
#include "headers/adafruitio.h"


#include <OneWire.h>
#include <DallasTemperature.h>
/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/
#include <ESP8266WiFi.h>


const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "djxmmx.net";
const uint16_t port = 17;

const uint8_t SENSOR = D1;

OneWire ds(SENSOR);

DeviceAddress sensor = { 0x28, 0xB5, 0x02, 0xF4, 0x0B, 0x00, 0x00, 0xCF };

// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(SENSOR);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

const int ledPin = D2;

void setup() {
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ReadSensorAddress();
}

void loop() {
  char msg[256];
  float tempC = ReadTemperature(sensor);
  float tempF = 1.8 * tempC + 32;
  sprintf(msg, "Temp: %f/C  %f/F\n", tempC, tempF);
  Serial.print(msg);
  delay(5000);
}

float ReadTemperature(DeviceAddress device) {
  digitalWrite(ledPin, HIGH);
  sensors.requestTemperatures();
  float temperature = sensors.getTempC(device);
  digitalWrite(ledPin, LOW);

  return temperature;
}

void ReadSensorAddress(void) {
  byte i;
  byte addr[8];
  
  if (!ds.search(addr)) {
    Serial.println(" No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  Serial.print(" ROM =");
  for (i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }
  Serial.write('\n');
}

void ConnectToSite(void)
{
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  // This will send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.println("hello from ESP8266");
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  delay(300000); // execute once every 5 minutes, don't flood remote service
}
