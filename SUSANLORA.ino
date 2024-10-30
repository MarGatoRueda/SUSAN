#include <Wire.h>
#include <Adafruit_AS7341.h>
#include <SoftwareSerial.h>

// Define pins for LEDs
const int ledPin1 = 3; // Refraction LED
const int ledPin2 = 4; // Bottom LED
const int ledPin3 = 5; // UV LED

const int intensity1 = 15;  // Refraction LED intensity (0-100)
const int intensity2 = 100; // Bottom LED intensity (0-100)
const int intensity3 = 100; // UV LED intensity (0-100)

int pwmValue1 = map(intensity1, 0, 100, 0, 255);
int pwmValue2 = map(intensity2, 0, 100, 0, 255);
int pwmValue3 = map(intensity3, 0, 100, 0, 255);

Adafruit_AS7341 as7341;
SoftwareSerial WIOE5(8, 9);  // RX, TX for WIO-E5 on pins D8 and D9

void setup() {
  Serial.begin(9600);     // Serial Monitor
  WIOE5.begin(9600);      // WIO-E5 serial setup

  // Initialize AS7341 sensor
  if (!as7341.begin()) {
    Serial.println("AS7341 sensor not found");
    while (1);
  }
  as7341.setATIME(1000);
  as7341.setASTEP(1200);
  as7341.setGain(AS7341_GAIN_8X);

  // Put sensor in standby initially
  as7341.enableSpectralMeasurement(false);

  // WIO-E5 setup commands
  WIOE5.println("+AT+MODE=TEST");
  delay(500);
  WIOE5.println("+AT+TEST=RFCFG,866,SF12,125,12,15,14,ON,OFF,OFF");
  delay(500);

  Serial.println("Setup complete. Listening for LoRa command...");
  WIOE5.println("+AT+TEST=RXLRPKT");
}

void loop() {
  // Check if data is available from the LoRa module
  if (WIOE5.available()) {
    String receivedLine = WIOE5.readStringUntil('\n');
    Serial.println("Received: " + receivedLine);
    
    // Check if the line contains the message
    if (receivedLine.startsWith("+TEST: RX ")) {
      // Extract the hex message part
      String hexMessage = receivedLine.substring(10);
      
      // Check if the message contains 'w'
      if (hexMessage = 277727) {
        Serial.println("Character 'w' detected!");
        wakeUpAndMeasure();
      }
    }
  }
}

void wakeUpAndMeasure() {
  as7341.setGain(AS7341_GAIN_8X);
  
  analogWrite(ledPin1, pwmValue1);  // Refraction LED on
  delay(2000);
  String dataString = getSensorData();
  analogWrite(ledPin1, 0);
  Serial.println("Refraction LED measurements done.");

  analogWrite(ledPin2, pwmValue2);  // Bottom LED on
  delay(2000);
  for (int i = 0; i < 6; i++) {
    dataString += getSensorData();
    delay(500);
  }
  analogWrite(ledPin2, 0);

  as7341.setGain(AS7341_GAIN_128X);
  analogWrite(ledPin3, pwmValue3);  // UV LED on
  delay(2000);
  for (int i = 0; i < 4; i++) {
    dataString += getSensorData();
    sendData(dataString);
    delay(500);
  }
  analogWrite(ledPin3, 0);
  Serial.println("Measurement complete");

  as7341.enableSpectralMeasurement(false);  // Standby mode

    // Send collected data via WIO-E5
}

String getSensorData() {
  uint16_t readings[12];
  String data = "";
  if (!as7341.readAllChannels(readings)) {
    Serial.println("Error reading sensor data");
    return data;
  }
  
  data += String(readings[0]) + ", " + String(readings[1]) + ", ";
  data += String(readings[2]) + ", " + String(readings[3]) + ", ";
  data += String(readings[6]) + ", " + String(readings[7]) + ", ";
  data += String(readings[8]) + ", " + String(readings[9]) + ", ";
  data += String(readings[10]) + ", " + String(readings[11]) + "\n";
  
  Serial.print(data);  // For troubleshooting
  return data;
}

void sendData(String data) {

  WIOE5.print("+AT+TEST=TXLRSTR,\"" + data + "\"\r\n");
  Serial.println("Data sent to server: " + data);
}