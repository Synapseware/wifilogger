#include <util.h>
#include <EthernetServer.h>
#include <Ethernet2.h>
#include <Dns.h>
#include <EthernetClient.h>
#include <EthernetUdp2.h>
#include <Dhcp.h>
#include <AdafruitIO_Time.h>
#include <AdafruitIO_Definitions.h>
#include <AdafruitIO_Dashboard.h>
#include <AdafruitIO_Data.h>
#include <AdafruitIO_Feed.h>
#include <AdafruitIO_MQTT.h>
#include <AdafruitIO_Group.h>
#include <AdafruitIO.h>
#include <AdafruitIO_WiFi.h>


#include "headers/credentials.h"
#include "headers/adafruitio.h"
#include <OneWire.h>
#include <DallasTemperature.h>



const char* ssid     = STASSID;
const char* password = STAPSK;

//const char* host = "djxmmx.net";
//const uint16_t port = 17;

const uint8_t SENSOR = D1;
const uint8_t LED_PIN = D2;
const uint8_t ERR_PIN = D3;

char msg[256];

OneWire ds(SENSOR);

DeviceAddress sensor = { 0x28, 0xB5, 0x02, 0xF4, 0x0B, 0x00, 0x00, 0xCF };

// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(SENSOR);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Connect to IO platform
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, STASSID, STAPSK);

// Setup feeds
AdafruitIO_Feed * temperatureFeed = io.feed("temperature");




//-----------------------------------------------------------------------------
// Configure hardware and initial state
void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(ERR_PIN, OUTPUT);

  digitalWrite(LED_PIN, 0);
  digitalWrite(ERR_PIN, 0);

  Serial.begin(115200);
  delay(1000);

  sprintf(msg, "WiFi info.  SSID=\"%s\", Password=\"%s\"", STASSID, STAPSK);
  Serial.println(msg);

  Serial.print("Connecting to AdafruitIO");
  io.connect();
  digitalWrite(ERR_PIN, 1);
  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  digitalWrite(ERR_PIN, 0);

  Serial.println();
  Serial.println(io.statusText());
  Serial.println();

  ReadSensorAddress();
}


//-----------------------------------------------------------------------------
// 
void loop()
{
  io.run();

  float tempC = ReadTemperature(sensor);
  float tempF = 1.8 * tempC + 32;
  temperatureFeed->save(tempC);
  sprintf(msg, "Temp: %f/C  %f/F\n", tempC, tempF);
  Serial.print(msg);
  delay(5000);
}


//-----------------------------------------------------------------------------
// 
float ReadTemperature(DeviceAddress device)
{
  digitalWrite(LED_PIN, HIGH);
  sensors.requestTemperatures();
  float temperature = sensors.getTempC(device);
  digitalWrite(LED_PIN, LOW);

  return temperature;
}


//-----------------------------------------------------------------------------
// Reads the sensors address(es) and prints them to the console
void ReadSensorAddress(void)
{
  byte i;
  byte addr[8];
  
  Serial.println("Discovering temperature sensors...");
  if (!ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  Serial.print(" ROM: ");
  for (i = 0; i < 8; i++) {
    sprintf(msg + i * 6, "0x%02x, ", addr[i]);
  }
  int len = strlen(msg);
  msg[len-2] = '\0';
  Serial.println(msg);
}
