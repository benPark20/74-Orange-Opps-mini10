#define PIN_TRIG 26
#define PIN_ECHO 25
#include <Adafruit_NeoPixel.h>
#define LED_PIN 17
#include <Alfredo_NoU2.h>
NoU_Servo servo(1);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
}

void loop() {
  int servoAngle = 90;
  // put your main code here, to run repeatedly:
  servo.write(servoAngle);
  delay(10); // this speeds up the simulation
}
