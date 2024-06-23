#include <Wire.h>
#include <Adafruit_AS7341.h>


#define BACKLIGHT_LED_PIN 6
#define REFRACTIVE_LED_PIN 3

Adafruit_AS7341 as7341;

void setup() {
  Serial.begin(115200);
  pinMode(BACKLIGHT_LED_PIN, OUTPUT);
  pinMode(REFRACTIVE_LED_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_LED_PIN, LOW);
  digitalWrite(REFRACTIVE_LED_PIN, LOW);

  if (!as7341.begin()) {
    Serial.println("AS7341 sensor not found");
    while (1);
  }

// Put the sensor into standby mode initially
  as7341.enableSpectralMeasurement(false);
}

void loop() {
  // Check for a command from the serial monitor
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'W') {
      wakeUpAndMeasure();
    }
  }
}


void wakeUpAndMeasure() {
  // Turn on the refractive LED and take a reading
  digitalWrite(REFRACTIVE_LED_PIN, HIGH);
  delay(2000);
  Serial.println("Refractive LED readings:");
  printSensorData();
  digitalWrite(REFRACTIVE_LED_PIN, LOW);

  // Turn on the backlight LED and take 10 readings
  digitalWrite(BACKLIGHT_LED_PIN, HIGH);
  Serial.println("Backlight LED readings:");
  for (int i = 0; i < 10; i++) {
    printSensorData();
    delay(500); // Adjust delay if needed
  }
  digitalWrite(BACKLIGHT_LED_PIN, LOW);

  // Send confirmation and go back to sleep
  Serial.println("Done");
  // Turn off the sensor and LEDs to save power
  as7341.enableSpectralMeasurement(false);
  digitalWrite(REFRACTIVE_LED_PIN, LOW);
  digitalWrite(BACKLIGHT_LED_PIN, LOW);
}

void printSensorData() {
    uint16_t readings[12];
    if (!as7341.readAllChannels(readings)) {
        Serial.println("Error reading sensor data");
        return;
    }
    
    for (int i = 0; i < 12; i++) {
        Serial.print(readings[i]);
        Serial.print(" ");
    }
    Serial.println();
}
