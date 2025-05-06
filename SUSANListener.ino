// Script for an ESP8266 to listen in and decode SUSANLORA's messages.

#include <SoftwareSerial.h>

// SoftwareSerial for Wio E5 (RX, TX)
SoftwareSerial wioSerial(14, 12); // ESP8266 RX (GPIO14/D5) -> Wio TX, ESP8266 TX (GPIO12/D6) -> Wio RX

void setup() {
  // Initialize USB Serial for debugging/output
  Serial.begin(115200);
  // Initialize SoftwareSerial for Wio E5
  wioSerial.begin(9600);

  // Wait for Serial to stabilize
  delay(1000);
  Serial.println("ESP8266 LoRa Receiver Starting...");

  // Initialize Wio E5
  initLoRa();
}

void loop() {
  // Check for incoming data from Wio E5
  if (wioSerial.available()) {
    String received = wioSerial.readStringUntil('\n');
    received.trim();

    // Look for +TEST: RX messages (second line with hex data)
    if (received.startsWith("+TEST: RX ")) {
      // Extract the hex payload (format: +TEST: RX "HEXDATA")
      int quoteStart = received.indexOf('"') + 1;
      int quoteEnd = received.lastIndexOf('"');
      if (quoteStart > 0 && quoteEnd > quoteStart) {
        String hexData = received.substring(quoteStart, quoteEnd);

        // Convert hex to ASCII string
        String asciiData = hexToAscii(hexData);

        // Print to USB Serial
        Serial.println(asciiData);
      } else {
        Serial.println("Error: Invalid RX format");
      }
    }
    // Optionally print metadata line for debugging
    else if (received.startsWith("+TEST: LEN ")) {
      Serial.println("Metadata: " + received);
    }
  }
}

void initLoRa() {
  // Configure Wio E5 for test mode and RF settings
  wioSerial.println("AT+MODE=TEST");
  delay(1000);
  wioSerial.println("AT+TEST=RFCFG,915,SF12,125,12,15,14,ON,OFF,OFF");
  delay(1000);
  // Set to receive mode
  wioSerial.println("AT+TEST=RXLRPKT");
  delay(1000);
  Serial.println("LoRa Receiver Initialized");
}

String hexToAscii(String hex) {
  String ascii = "";
  // Ensure hex string length is even
  if (hex.length() % 2 != 0) {
    Serial.println("Invalid hex string length");
    return ascii;
  }

  // Convert pairs of hex digits to ASCII characters
  for (unsigned int i = 0; i < hex.length(); i += 2) {
    String hexPair = hex.substring(i, i + 2);
    char c = (char)strtol(hexPair.c_str(), NULL, 16);
    ascii += c;
  }
  return ascii;
}