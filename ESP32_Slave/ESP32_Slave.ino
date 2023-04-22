#include <Arduino.h>
#include <Wire.h>
#include <WireSlave.h>
#include <string>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

#include <ESPAsyncWebServer.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_SLAVE_ADDR 0x04

// define prototypes
void receiveEvent(int howMany);
void appendFile(fs::FS &fs, const char * path, String message);
void sendReadingsToBackend();

const char *ssid = "VM0190593";
const char *password = "rmMyvxjp7spv";

const char *soft_ap_ssid = "SmartWirelessSensorAP";
const char *soft_ap_password = "testpassword";

AsyncWebServer server(80);

const char *ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

float voltage = 0.0;
float current = 0.0;
float frequency = 0.0;
float power = 0.0;

float avgVoltage = 0.0;
float avgCurrent = 0.0;
float avgFrequency = 0.0;
float avgPower = 0.0;

float energyConsumption = 0.0;
float faultCounter = 0.0;
float timeElapsed = 0.0;
float offset = 0.0;

void setup()
{
  Serial.begin(2000000);

  // I2C
  bool success = WireSlave.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR);
  if (!success) {
    Serial.println("I2C slave init failed");
    ESP.restart();
  }

  // microSD card
  while (!SD.begin()) {
    delay(100);
    Serial.println("Failed to initialise microSD card - check that it's inserted");
    ESP.restart();
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Invalid card type: none");
    ESP.restart();
  }

  // Wi-Fi server mode
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  WiFi.begin(ssid, password);

  // Connect to Wi-Fi network


  Serial.println("Connecting");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - startTime > 3000) {
      Serial.println("***WiFi Disconnected - Please try restarting the ESP32***");
      Serial.println("The sensor readings are still available via the Smart Wireless Sensor Access Point");
      Serial.print("Connect to the \"SmartWirelessSensorAP\" Access Point on any device, then go to http://");
      Serial.print(WiFi.softAPIP());
      Serial.println("/readings");
      break;
    }
  }
  Serial.println("");

  WireSlave.onReceive(receiveEvent);

  // Time - Use Wi-Fi to syncronise the ESP32 time with this server (only needs done on initialisation)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  // Wi-Fi Server and AP Server Setup
  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());

  Serial.print("ESP32 IP on the WiFi network: ");
  Serial.println(WiFi.localIP());

  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest * request) {
    const char * format =
      "Smart Wireless Sensor Measurements\n"
      "----------------------------------\n"
      "Voltage: %.3fkV\n"
      "Current: %.3fA\n"
      "Frequency: %.3fHz\n"
      "Power Consumption: %.3fkW\n"
      "Average Voltage: %.3fkV\n"
      "Average Current: %.3fA\n"
      "Average Frequency: %.3fHz\n"
      "Average Power Consumption: %.3fkW\n"
      "Energy Consumption: %.3fkWh\n"
      "Number of Faults: %.0f\n"
      "Time Elapsed: %.0fs\n"
      "Voltage Offset/Midpoint: %.3fV\n";


    char message[450];
    snprintf(
      message,
      450,
      format,

      voltage,
      current,
      frequency,
      power,

      avgVoltage,
      avgCurrent,
      avgFrequency,
      avgPower,

      energyConsumption,
      faultCounter,
      timeElapsed,
      offset
    );

    if (ON_STA_FILTER(request)) {
      request->send(200, "text/plain", message);
      return;

    } else if (ON_AP_FILTER(request)) {
      request->send(200, "text/plain", message);
      return;
    }
  });

  server.begin();
}

void loop()
{
  WireSlave.update();
  delay(1);
}

void sendReadingsToBackend() {
  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    char requestURL[250];
    snprintf(
      requestURL,
      250,
            "https://smart-wireless-sensor-backend.nw.r.appspot.com/setReadings/%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
//      "http://192.168.0.26:8081/setReadings/%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
      voltage,
      current,
      frequency,
      power,

      avgVoltage,
      avgCurrent,
      avgFrequency,
      avgPower,

      energyConsumption,
      faultCounter,
      timeElapsed,
      offset
    );

    String serverPath = requestURL;

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
    Serial.print("Sending GET request to: ");
    Serial.println(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {

    Serial.println("***WiFi Disconnected - Please try restarting the ESP32***");
    Serial.println("The sensor readings are still available via the Smart Wireless Sensor Access Point");
    Serial.print("Connect to the \"SmartWirelessSensorAP\" Access Point on any device, then go to http://");
    Serial.print(WiFi.softAPIP());
    Serial.println("/readings");
  }

}


void receiveEvent(int howMany)
{
  String s = "";
  while (WireSlave.available()) // loop through every byte in the I2C message
  {
    char ch = WireSlave.read();
    s += ch;
  }

  //  Serial.println(s);

  int commaCount = 0;
  int start = 0;
  for (int i = 0; i < s.length(); i++) {
    char ch = s.charAt(i);
    if (ch == ',') {
      float floatValue = s.substring(start, i).toFloat();
      switch (commaCount) {
        case 0:
          voltage = floatValue / 1000.0;
          break;
        case 1:
          current = floatValue;
          break;
        case 2:
          frequency = floatValue;
          break;
        case 3:
          power = floatValue / 1000.0;
          break;

        case 4:
          avgVoltage = floatValue / 1000.0;
          break;
        case 5:
          avgCurrent = floatValue;
          break;
        case 6:
          avgFrequency = floatValue;
          break;
        case 7:
          avgPower = floatValue / 1000.0;
          break;

        case 8:
          energyConsumption = floatValue;
          break;
        case 9:
          faultCounter = floatValue;
          break;
        case 10:
          timeElapsed = floatValue;
          break;
        case 11:
          offset = floatValue;
          break;
      }
      start = i + 1;
      commaCount++;
    }
  }

  const char * format =
    "-----------------------------------------------------------\n"
    "Voltage: %.3fkV\n"
    "Current: %.3fA\n"
    "Frequency: %.3fHz\n"
    "Power Consumption: %.3fkW\n"
    "Average Voltage: %.3fkV\n"
    "Average Current: %.3fA\n"
    "Average Frequency: %.3fHz\n"
    "Average Power Consumption: %.3fkW\n"
    "Energy Consumption: %.3fkWh\n"
    "Number of Faults: %.0f\n"
    "Time Elapsed: %.0fs\n"
    "Voltage Offset/Midpoint: %.3fV\n";


  char message[450];
  snprintf(
    message, 450, format,

    voltage,
    current,
    frequency,
    power,

    avgVoltage,
    avgCurrent,
    avgFrequency,
    avgPower,

    energyConsumption,
    faultCounter,
    timeElapsed,
    offset
  );

  struct tm timeinfo;
  char dateTimeString[] = "/Smart_Wireless_Sensor_Readings.txt";
  if (getLocalTime(&timeinfo)) {
    strftime(
      dateTimeString,
      100,
      //      "/Smart_Wireless_Sensor_Readings/%Y/%B/%d/%H%M/%d%B%Y_%H%M.txt",
      "/%d%B%Y_%H%M.txt",
      &timeinfo
    );
  } else {
    Serial.println("Failed to obtain time, appending to Smart_Wireless_Sensor_Readings.txt file");
  }

  const char * path = dateTimeString;
  appendFile(SD, path, message);

  sendReadingsToBackend();
}

void appendFile(
  fs::FS &fs,
  const char * path,
  String message
) {

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Text file could not be appended to or created");
    return;
  }

  Serial.print(message);
  if (!file.print(message)) {
//    Serial.println("***File append failed: Check that microSD card has been inserted, then reboot***");
  }

  file.close();
}
