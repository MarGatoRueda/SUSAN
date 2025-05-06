// SUSAN script to send autonomously data every 3 minutes at random over LoRa

#include <Wire.h>
#include <Adafruit_AS7341.h>

// Define the pins for the LEDs
const int ledPin1 = 3; // Refraction LED
const int ledPin2 = 6; // Bottom LED
const int ledPin3 = 5; // UV LED

const int intensity1 = 15; // Set the intensity for LED 1 (0-100): Refraction LED
const int intensity2 = 100; // Set the intensity for LED 2 (0-100): Bottom LED
const int intensity3 = 100; // Set the intensity for LED 3 (0-100): UV LED

// Map the intensity values from 0-100 to the range of 0-255 (PWM range)
int pwmValue1 = map(intensity1, 0, 100, 0, 255);
int pwmValue2 = map(intensity2, 0, 100, 0, 255);
int pwmValue3 = map(intensity3, 0, 100, 0, 255);

Adafruit_AS7341 as7341;

// Timing variables
const unsigned long intervalMin = 30000; // Minimum 30 seconds
const unsigned long intervalMax = 180000; // Maximum 3 minutes
unsigned long lastMeasurement = 0;
unsigned long nextMeasurementDelay = 0;

void setup() {
  // Initialize hardware Serial for Wio E5
  Serial.begin(9600); // Wio E5 baud rate

  // Initialize random seed
  randomSeed(analogRead(0));

  // Initialize AS7341
  if (!as7341.begin()) {
    while (1); // Halt if sensor not found
  }
  as7341.setATIME(1000);
  as7341.setASTEP(1200);
  as7341.setGain(AS7341_GAIN_8X);
  as7341.enableSpectralMeasurement(false);

  // Initialize Wio E5
  initLoRa();

  // Set first random delay (30 to 180 seconds)
  nextMeasurementDelay = random(intervalMin, intervalMax);
}

void loop() {
  unsigned long currentTime = millis();

  // Check if it's time to take a measurement
  if (currentTime - lastMeasurement >= nextMeasurementDelay) {
    wakeUpAndMeasure();
    lastMeasurement = currentTime;
    // Set next random delay (30 to 180 seconds)
    nextMeasurementDelay = random(intervalMin, intervalMax);
  }
}

void initLoRa() {
  // Configure Wio E5 for test mode and RF settings
  Serial.println("AT+MODE=TEST");
  delay(1000);
  Serial.println("AT+TEST=RFCFG,915,SF12,125,12,15,14,ON,OFF,OFF");
  delay(1000);
}

void wakeUpAndMeasure() {
  as7341.setGain(AS7341_GAIN_8X);
  // Turn on the refractive LED and take a reading
  analogWrite(ledPin1, pwmValue1);
  delay(2000);
  sendSensorData("Refractive");
  analogWrite(ledPin1, 0);

  // Turn on the backlight LED and take 6 readings
  analogWrite(ledPin2, pwmValue2);
  delay(2000);
  for (int i = 0; i < 6; i++) {
    sendSensorData("Backlight");
    delay(1000);
  }
  analogWrite(ledPin2, 0);

  // Increase gain to 128x for UV LED
  as7341.setGain(AS7341_GAIN_128X);
  // Turn on the UV LED and take 4 readings
  analogWrite(ledPin3, pwmValue3);
  delay(2000);
  for (int i = 0; i < 4; i++) {
    sendSensorData("UV");
    delay(1000);
  }
  analogWrite(ledPin3, 0);

  as7341.enableSpectralMeasurement(false);
}

void sendSensorData(String mode) {
  uint16_t readings[12];
  if (!as7341.readAllChannels(readings)) {
    return; // Skip if sensor read fails
  }

  // Format data as CSV string
  String data = mode + "," + 
                String(readings[0]) + "," + // ADC0/F1 415nm
                String(readings[1]) + "," + // ADC1/F2 445nm
                String(readings[2]) + "," + // ADC2/F3 480nm
                String(readings[3]) + "," + // ADC3/F4 515nm
                String(readings[6]) + "," + // ADC0/F5 555nm
                String(readings[7]) + "," + // ADC1/F6 590nm
                String(readings[8]) + "," + // ADC2/F7 630nm
                String(readings[9]) + "," + // ADC3/F8 680nm
                String(readings[10]) + "," + // ADC4/Clear
                String(readings[11]); // ADC5/NIR

  // Convert to hex for LoRa transmission
  String hexData = "";
  for (unsigned int i = 0; i < data.length(); i++) {
    char hex[3];
    sprintf(hex, "%02X", data[i]);
    hexData += hex;
  }

  // Send over LoRa
  Serial.print("AT+TEST=TXLRPKT,\"");
  Serial.print(hexData);
  Serial.println("\"");
  delay(500); // Wait for transmission to complete
}