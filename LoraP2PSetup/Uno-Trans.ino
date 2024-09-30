#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"

// Pin definitions
#define DHTPIN 3            // Pin where the DHT22 is connected
#define DHTTYPE DHT22       // DHT 22 (AM2302) sensor type
#define SOIL_MOISTURE_PIN A0 // Analog pin for soil moisture sensor

// Create instances
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize DHT22 sensor
  dht.begin();

  // Initialize LoRa
  if (!LoRa.begin(868E6)) {  // Set frequency to 868 MHz
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Transmitter initialized.");
}

void loop() {
  // Reading temperature and humidity from DHT22 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Reading soil moisture value
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  
  // Map soil moisture value from 175 (99% wet) to 610 (1% dry) into a percentage
  float soilMoisturePercent = map(soilMoistureValue, 175, 610, 99, 1);
  soilMoisturePercent = constrain(soilMoisturePercent, 1, 99);  // Ensure moisture is within valid range

  // Check if the DHT sensor readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Format the data with labels
  String dataToSend = "Temp:" + String(temperature, 2) + ",Hum:" + String(humidity, 2) + ",SoilMoisture:" + String(soilMoisturePercent, 2);

  // Debugging output: print sensor values to serial monitor
  Serial.println("Sending data: " + dataToSend);

  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.print(dataToSend);
  LoRa.endPacket();

  // Wait 2 seconds before sending the next reading
  delay(2000);
}
