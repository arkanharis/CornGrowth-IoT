#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#define DPIN 4         // Pin to connect DHT sensor (GPIO number)
#define DTYPE DHT11    // Define DHT 11 or DHT22 sensor type
#define SOILPIN 34      // Pin to connect Soil Moisture sensor (GPIO number)

const char* ssid = "Redmi Note 11";
const char* password = "123456789";
const char* serverUrl = "http://192.168.33.31:8000/data";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Update time every 60 seconds

DHT dht(DPIN, DTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize NTPClient to get time
  timeClient.begin();
}

void loop() {
  delay(2000);

  timeClient.update(); // Update the time from NTP server

  float tc = dht.readTemperature(false);  // Read temperature in C
  float hu = dht.readHumidity();          // Read Humidity
  int soilMoistureValue = analogRead(SOILPIN); // Read Soil Moisture

  // Get current timestamp
  unsigned long timestamp = timeClient.getEpochTime(); // Unix timestamp
  String timestampStr = String(timestamp); // Convert Unix timestamp to string

  // Create JSON string
  String json = "{\"temperature\": ";
  json += String(tc);
  json += ", \"humidity\": ";
  json += String(hu);
  json += ", \"soil_moisture\": ";
  json += String(soilMoistureValue);
  json += ", \"timestamp\": \"";
  json += timestampStr;
  json += "\"}";

  Serial.println(json);

  // Send JSON to server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(json);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
