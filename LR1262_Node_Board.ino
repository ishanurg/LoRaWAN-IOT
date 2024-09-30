#include <Arduino.h>
#include <DHT.h>

#define DEBUG true

// Get DEVEUI, APPEUI, and APPKEY from The Things Network
#define DEVEUI ""
#define APPEUI ""
#define APPKEY ""

// Pin Definitions
#define TX_PIN 0       // Serial TX Pin
#define RX_PIN 1       // Serial RX Pin
#define RESET_PIN 6    // LoRa RESET Pin
#define DIO1_PIN 7     // LoRa DIO1 Pin
#define DIO2_PIN 8     // LoRa DIO2 Pin

// SPI Pins
#define SPI_SCK_PIN 2  // SPI Clock
#define SPI_MOSI_PIN 3 // SPI Master Out Slave In
#define SPI_MISO_PIN 4 // SPI Master In Slave Out
#define SPI_CS_PIN 5   // SPI Chip Select

// Soil Moisture Sensor Pin
#define SOIL_MOISTURE_PIN 26 // Analog pin connected to the soil moisture sensor

// DHT22 Sensor
#define DHTPIN 3     // Digital pin connected to the DHT22 sensor
#define DHTTYPE DHT22  // DHT 22 (AM2302) sensor

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
    // Initialize Serial and Serial1
    Serial.begin(9600);          // Serial for communication with the PC
    Serial1.begin(9600);         // Serial1 for communication with the LoRa module

    Serial.println("Initializing LoRaWAN module. Please wait.");

    pin_init();
    dht.begin(); // Initialize DHT22 sensor

    Serial.println("Initialization complete");

    // Send AT commands to configure the LoRaWAN module
    sendData("AT+RFS", 3000, DEBUG);
    sendData("AT+ChannelMode=0", 3000, DEBUG);
    sendData("AT+RESET", 3000, DEBUG);
    sendData("AT+BAND=5", 3000, DEBUG); // Assuming Band 5 corresponds to EU868 or similar
    sendData("AT+ChannelMode=0", 3000, DEBUG); // Set channel mode (0: default)

    // Set DEVEUI, APPEUI, and APPKEY
    sendData("AT+DevEui=" + String(DEVEUI), 3000, DEBUG);
    sendData("AT+AppEui=" + String(APPEUI), 3000, DEBUG);
    sendData("AT+AppKey=" + String(APPKEY), 3000, DEBUG);

    // Join LoRaWAN network
    sendData("AT+JOIN=1,2", 5000, DEBUG);
    delay(30000);
}

void loop()
{
    // Read soil moisture sensor value
    int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
    int moisturePercent = (int) mapToPercentage(soilMoistureValue);  // Convert to integer

    // Clamp the percentage value to be between 1% and 99%
    if (moisturePercent < 1) {
        moisturePercent = 1;
    } else if (moisturePercent > 99) {
        moisturePercent = 99;
    }

    // Read DHT22 sensor values
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Print the values to the Serial terminal
    Serial.print("Soil Moisture: ");
    Serial.print(moisturePercent);   // No decimals for soil moisture
    Serial.print("%, Temperature: ");
    Serial.print(temperature);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    // Encode soil moisture, temperature, and humidity as bytes
    uint8_t moistureByte = (uint8_t)(moisturePercent); // No scaling needed
    uint16_t tempValue = (uint16_t)(temperature * 10);   // Scale to 1 decimal place
    uint16_t humidityValue = (uint16_t)(humidity * 10);  // Scale to 1 decimal place

    // Create the payload to send (5 bytes: 1 byte for soil moisture, 2 bytes for temperature, 2 bytes for humidity)
    uint8_t payload[5];
    payload[0] = moistureByte;                        // Integer soil moisture
    payload[1] = (tempValue >> 8) & 0xFF;             // Temperature high byte
    payload[2] = tempValue & 0xFF;                    // Temperature low byte
    payload[3] = (humidityValue >> 8) & 0xFF;         // Humidity high byte
    payload[4] = humidityValue & 0xFF;                // Humidity low byte

    // Convert the payload to a hex string
    String payloadString = "";
    for (int i = 0; i < 5; i++) {
        if (payload[i] < 16) {
            payloadString += "0";  // Add leading zero for single hex digits
        }
        payloadString += String(payload[i], HEX);
    }

    // Send the payload to TTN
    sendData("AT+SEND=1:1:" + payloadString, 3000, DEBUG);

    delay(60000); // Wait 1 minute before sending the next value
}

void pin_init()
{
    pinMode(RESET_PIN, OUTPUT);
    pinMode(DIO1_PIN, INPUT);
    pinMode(DIO2_PIN, INPUT);

    // Initialize pins
    digitalWrite(RESET_PIN, LOW);
    delay(100);   // Short delay
    digitalWrite(RESET_PIN, HIGH);
    delay(1000);  // Allow time for the module to reset and initialize
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    Serial1.println(command);
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            response += c;
        }
    }
    if (debug)
    {
        Serial.print(response);
    }
    return response;
}

float mapToPercentage(int sensorValue)
{
    // Map sensor values to percentage between 1% and 99%
    float minSensorValue = 600;
    float maxSensorValue = 325;
    float minPercentage = 1.0;
    float maxPercentage = 99.0;
    
    // Calculate percentage
    float percentage = (sensorValue - minSensorValue) * (maxPercentage - minPercentage) / (maxSensorValue - minSensorValue) + minPercentage;

    return percentage;
}
