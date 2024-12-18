#include <Wire.h>
#include <Adafruit_AS7341.h>

// Define the pins for the LEDs
const int ledPin1 = 3; // Refraction LED
const int ledPin2 = 4; // Bottom LED
const int ledPin3 = 5; // UV LED

const int intensity1 = 15; // Set the intensity for LED 1 (0-100): Refraction LED
const int intensity2 = 100; // Set the intensity for LED 2 (0-100): Bottom LED
const int intensity3 = 100; // Set the intensity for LED 3 (0-100): UV LED

bool msg = false;

// Map the intensity values from 0-100 to the range of 0-255 (PWM range)
int pwmValue1 = map(intensity1, 0, 100, 0, 255);
int pwmValue2 = map(intensity2, 0, 100, 0, 255);
int pwmValue3 = map(intensity3, 0, 100, 0, 255);

Adafruit_AS7341 as7341;

void setup() {
  Serial.begin(115200);

  if (!as7341.begin()) {
    Serial.println("AS7341 sensor not found");
    while (1);
  }
  as7341.setATIME(1000);
  as7341.setASTEP(1200);
  as7341.setGain(AS7341_GAIN_8X);


// Put the sensor into standby mode initially
  as7341.enableSpectralMeasurement(false);
}

void loop() {
  // Check for a command from the serial monitor
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'w') {
      wakeUpAndMeasure();
    }
    else if (command == 'p') {
      PWMTest();
  }
}
}

void PWMTest() {
  // Using the refractive LED, cycle through the intensity values from 0-100 of pwmValue1, one step every 2 seconds.
  // Print the sensor data for each intensity value.
  for (int i = 0; i <= 100; i++) {
    pwmValue2 = map(i, 0, 100, 0, 255);
    analogWrite(ledPin2, pwmValue2);
    delay(2000);
    printSensorData();
  }
  analogWrite(ledPin2, 0);
  Serial.println("Done");
}

void wakeUpAndMeasure() {
  
  as7341.setGain(AS7341_GAIN_8X);
  // Turn on the refractive LED and take a reading
  analogWrite(ledPin1, pwmValue1);
  delay(2000);
  printSensorData();
  analogWrite(ledPin1, 0);
  Serial.println("Refractive LED measurements done.");

  // Turn on the backlight LED and take 6 readings
  analogWrite(ledPin2, pwmValue2);
  delay(2000);
  for (int i = 0; i < 6; i++) {
    printSensorData();
    delay(500);
  }
  analogWrite(ledPin2, 0);

  Serial.println("Changing to UV measurements");
  // Increase gain to 128x for UV LED
  as7341.setGain(AS7341_GAIN_128X);
  // Turn on the UV LED and take a 4 readings
  analogWrite(ledPin3, pwmValue3);
  delay(2000);
  for (int i = 0; i < 4; i++) {
    printSensorData();
    delay(500);
  }
  analogWrite(ledPin3, 0);


  Serial.println("Done");
  // Turn off the sensor and LEDs to save power
  as7341.enableSpectralMeasurement(false);
}

void printSensorData() {
    uint16_t readings[12];
    if (!as7341.readAllChannels(readings)) {
        Serial.println("Error reading sensor data");
        return;
    }
    
  Serial.print(readings[0]); //ADC0/F1 415nm
  Serial.print(", ");
  Serial.print(readings[1]); //ADC1/F2 445nm
  Serial.print(", ");
  Serial.print(readings[2]); //ADC2/F3 480nm
  Serial.print(", ");
  Serial.print(readings[3]); //ADC3/F4 515nm
  Serial.print(", ");
  Serial.print(readings[6]); //ADC0/F5 555nm
  Serial.print(", ");
  Serial.print(readings[7]); //ADC1/F6 590nm
  Serial.print(", ");
  Serial.print(readings[8]); //ADC2/F7 630nm
  Serial.print(", ");
  Serial.print(readings[9]); //ADC3/F8 680nm
  Serial.print(", ");
  Serial.print(readings[10]);//ADC4/Clear
  Serial.print(", ");
  Serial.print(readings[11]);//ADC5/NIR
  Serial.println(); //

}