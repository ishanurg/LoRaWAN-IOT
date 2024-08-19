#include <Arduino.h>

#define DEBUG true

// Get appeui and appkey from The Things Network
#define DEVEUI "70B3D57ED0069BC3"
#define APPEUI "9301456882123456"
#define APPKEY "73AF1097A6E38F0728BA77BB9AE5206B"

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

void setup()
{
    // Initialize Serial and Serial1
    Serial.begin(9600);          // Serial for communication with the PC
    Serial1.setRX(RX_PIN);       // Set RX pin for Serial1
    Serial1.setTX(TX_PIN);       // Set TX pin for Serial1
    Serial1.begin(9600);         // Serial1 for communication with the LoRa module

    Serial.println("Initializing LoRaWAN module. Please wait.");

    pin_init();
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
    sendData("AT+JOIN=1,2", 30000, DEBUG);

    // Send a test message to the network
    // Example: Sending "Hello World" in hexadecimal format
    sendData("AT+SEND=1:1:48656C6C6F20576F726C64", 3000, DEBUG); // "Hello World"
}

void loop()
{
    // Forward data from Serial to Serial1
    if (Serial.available())
    {
        Serial1.write(Serial.read());
    }

    // Forward data from Serial1 to Serial
    if (Serial1.available())
    {
        Serial.write(Serial1.read());
    }
}

void pin_init()
{
    pinMode(RESET_PIN, OUTPUT);
    pinMode(DIO1_PIN, INPUT);
    pinMode(DIO2_PIN, INPUT);

    // Initialize pins
    digitalWrite(RESET_PIN, LOW);
    delay(10);   // Short delay
    digitalWrite(RESET_PIN, HIGH);
    delay(100);  // Allow time for the module to reset and initialize
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
