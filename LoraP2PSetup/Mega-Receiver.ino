#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>

// Define SoftwareSerial pins for communication with NodeMCU
SoftwareSerial arduinoSerial(10, 11); // RX, TX (adjust the pins as necessary)

void setup() {
  Serial.begin(9600);          // Initialize serial communication for debugging
  arduinoSerial.begin(9600);   // Initialize SoftwareSerial for communication with NodeMCU

  while (!Serial); // Wait for Serial to be ready

  Serial.println("LoRa Receiver");

  // Initialize LoRa with frequency 868 MHz
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1); // Stay here if LoRa initialization fails
  }
}

void loop() {
  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Read the packet and convert to string
    String receivedValue = "";
    while (LoRa.available()) {
      receivedValue += (char)LoRa.read();
    }

    // Print the received data to Serial Monitor
    Serial.print("Received data: ");
    Serial.println(receivedValue);

    // Send the received data to NodeMCU via SoftwareSerial
    arduinoSerial.println(receivedValue);

    // Optional: Add more debugging output here to verify successful sending
    Serial.println("Data sent to NodeMCU.");
  }

  // Reduce delay if you want to process new packets more frequently
  delay(500); // Adjust delay based on your requirements
}
