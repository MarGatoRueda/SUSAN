#include <Adafruit_AS7341.h>

Adafruit_AS7341 as7341;

// Define the LED pins, D3 and D6, for the two LEDs
const int ledPin1 = 3; // Refraction LED
const int ledPin2 = 6; // Bottom LED

const int intensity1 = 100; // Set the intensity for LED 1 (0-100): Refraction LED
const int intensity2 = 100; // Set the intensity for LED 2 (0-100): Bottom LED
bool msg = false;

void setup() {
  // Set the LED pins as outputs
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  // Start the serial communication
  Serial.begin(115200);

  if (!as7341.begin()){
    Serial.println("No AS7341 found");
    while (1) {delay(10);}
  }
  as7341.setATIME(200);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_64X);
  
  pinMode(5, OUTPUT);

  // Turn on the refraction LED, turn off the bottom LED, wait for 5 seconds and record the data
  analogWrite(ledPin1, intensity1);
  analogWrite(ledPin2, 0);
  delay(5000);
}

void loop() {
  // Map the intensity values from 0-100 to the range of 0-255 (PWM range)
  int pwmValue1 = map(intensity1, 0, 100, 0, 255);
  int pwmValue2 = map(intensity2, 0, 100, 0, 255);
  
  // Set the PWM values to control LED intensities
  analogWrite(ledPin1, pwmValue1);
  analogWrite(ledPin2, pwmValue2);
  if (Serial.available()){
    char c = Serial.read();
    if (c == 'm'){
      msg = true;
      digitalWrite(5, HIGH);
      while (msg) {
        measure();
        c = Serial.read();
        if (c == 's'){
          msg = false;
          digitalWrite(5, LOW);
        }
      }
    }
  }
}

void measure(){
  uint16_t readings[12];

  if (!as7341.readAllChannels(readings)){
    Serial.println("Failed to read all channels");
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
  Serial.println(); //Newline for new data
}
