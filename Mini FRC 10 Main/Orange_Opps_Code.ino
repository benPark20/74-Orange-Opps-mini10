/*
TIPS FOR TUNING!
when axle is positioned right
Servo minimum is -20
Servo maximum is 130
Motors dont work until 0.6 to 0.7

*/


#include <PestoLink-Receive.h>
#include <Alfredo_NoU2.h>
#include <NewPing.h>

// If your robot has more than a drivetrain and one servo, add those actuators here 
NoU_Motor frontLeftMotor(1); 
NoU_Motor frontRightMotor(2);
NoU_Motor rearLeftMotor(3);
NoU_Motor rearRightMotor(4);
NoU_Motor indexerMotor(5);
NoU_Motor shooterMotor(6);
NoU_Servo servo(1);

// This creates the drivetrain object, you shouldn't have to mess with this
NoU_Drivetrain drivetrain(&frontLeftMotor, &frontRightMotor, &rearLeftMotor, &rearRightMotor);

//distance sensor
#define TRIGGER_PIN 25
#define ECHO_PIN 26
#define MAX_DISTANCE 400

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

float distance;

// This lets us easily keep track of whether the robot is in auto mode or not.
// Experiment with adding more modes!
enum State { MANUAL, AUTO };

// These are the buttons we look for.
int AUTO_START_BUTTON = 8;
int AUTO_CANCEL_BUTTON = 9;

// This stores the time at which we started auto mode. This lets us keep track of how long we've been in auto mode for.
long AUTO_START_TIME = 0;
long SHOOTER_START_TIME = 0;
State ROBOT_STATE;

void setup() {
  //EVERYONE SHOULD CHANGE "ESP32 Bluetooth" TO THE NAME OF THEIR ROBOT
  PestoLink.begin("CallingAllBluds");
  Serial.begin(9600);
  // If a motor in your drivetrain is spinning the wrong way, change the value for it here from 'false' to 'true'
  frontLeftMotor.setInverted(true);
  frontRightMotor.setInverted(false);
  rearLeftMotor.setInverted(true);
  rearRightMotor.setInverted(false);

  // No need to mess with this code
  RSL::initialize();
  RSL::setState(RSL_ENABLED);
  ROBOT_STATE = MANUAL;
}

void loop() {
  // Here we define the variables we use in the loop
  float indexerThrottle = 0;
  float shooterThrottle = 0.1; //this needs tuning
  int servoAngle = 90;
  int AutoAngle;

  //When PestoLink.update() is called, it returns "true" only if the robot is connected to phone/laptop  
  if (PestoLink.update()) {
    if (ROBOT_STATE == MANUAL) {
      // We only want to check manual controls if we're in manual mode!
      if (PestoLink.buttonHeld(AUTO_START_BUTTON)) {
        // If the auto mode button is pressed, we should switch to auto mode.
        ROBOT_STATE = AUTO;
        AUTO_START_TIME = millis();
        return; // We don't want to check the manual controls - we just switched to auto mode! This immediately restarts the loop.
      }
      float xVelocity = PestoLink.getAxis(0) * 1;
      float yVelocity = -PestoLink.getAxis(1) * 1;
      float rotation = PestoLink.getAxis(2) * 1;

      drivetrain.holonomicDrive(xVelocity, yVelocity, rotation);

      // Here we decide what the servo angle will be based on if a button is pressed ()

      // Servo Code:
      if (PestoLink.buttonHeld(12)) {
        servoAngle = 130; //climber up angle/amp
        servo.write(servoAngle);
      } else if (PestoLink.buttonHeld(13)) {
        servoAngle = 29; //subwoofer angle
        servo.write(servoAngle);
      } else if (PestoLink.buttonHeld(15)) {
        servoAngle = 53; //podium
        servo.write(servoAngle);
      } else if (PestoLink.buttonHeld(2)) {
        distance = sonar.ping()/10;
        distance += 20; //comment out this line for no subwoofer. 20cm = 8 inches
        Serial.println(distance);
        if (distance != 0.00){
          AutoAngle = distance/5;
          servoAngle = AutoAngle;
          servo.write(servoAngle);
          shooterThrottle = -1;
          AutoAngle = distance/5;
          servoAngle = AutoAngle;
          servo.write(servoAngle);
      }
      //Manual Servo Control
      
      if ((PestoLink.getRawAxis(3)) > 230){
        if (servo.getDegrees() > 0){
          servo.write(servo.getDegrees() - 0.001);
        }
      }

      // Motor Code:
      if (PestoLink.buttonHeld(4)) {
        indexerThrottle = 1;
        servo.write(-0);
      } else if (PestoLink.buttonHeld(6)) {
        indexerThrottle = -1;
      } 
      else {
        indexerThrottle = 0;
      }
      
      if (PestoLink.buttonHeld(5)) {
        shooterThrottle = 1;
      } else if (PestoLink.buttonHeld(0)) {
        shooterThrottle = -1;
        SHOOTER_START_TIME = millis();
      } else if (PestoLink.buttonHeld(7)){
      distance = sonar.ping()/10;
      distance += 20; //comment out this line for no subwoofer. 20cm = 8 inches
      Serial.println(distance);
      if (distance != 0.00){
        AutoAngle = distance/5;
        servoAngle = AutoAngle;
        servo.write(servoAngle);
        shooterThrottle = -1;
        AutoAngle = distance/5;
        servoAngle = AutoAngle;
        servo.write(servoAngle);  
        shooterThrottle = -1;
        SHOOTER_START_TIME = millis();
      } else {
        if (PestoLink.buttonHeld(4) || PestoLink.buttonHeld(6)){
          shooterThrottle = 0.8;
        } else{
          shooterThrottle = 0;
        }
      }
      
      // Handle indexer motor timing based on SHOOTER_START_TIME
      if (SHOOTER_START_TIME != 0) {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - SHOOTER_START_TIME;
        if (elapsedTime > 1 && elapsedTime <= 1250) {
          indexerThrottle = 1;
        } else if (elapsedTime > 1500) {
          SHOOTER_START_TIME = 0; // Reset SHOOTER_START_TIME after the interval
        }
      }

      indexerMotor.set(indexerThrottle);
      shooterMotor.set(shooterThrottle);
    } else {
      // We're in auto mode, so we should handle auto mode.
      if (PestoLink.buttonHeld(AUTO_CANCEL_BUTTON)) {
        // Check to see if we should cancel auto mode.
        ROBOT_STATE = MANUAL;
        return;
      }
      
      if ((millis() - AUTO_START_TIME) < 1000) {
        servo.write(33);
        shooterMotor.set(-1);
        if (TSA > 750){
          indexerMotor.set(1);
        }
      } else if(((millis() - AUTO_START_TIME) > 999) && ((millis() - AUTO_START_TIME) < 3500)){
        if(TSA < 2750) {
          servo.write(-20);
          drivetrain.arcadeDrive(1, 0);
          shooterMotor.set(1);
        } else {
          drivetrain.arcadeDrive(-1, 0);
          indexerMotor.set(0);
          shooterMotor.set(0);
        }
      } else if(((millis() - AUTO_START_TIME) > 3499) && ((millis() - AUTO_START_TIME) < 6000)){
        servo.write(33);
        shooterMotor.set(-1);
        if ((millis() - AUTO_START_TIME) > 4000){
          indexerMotor.set(1);
        } else {
          indexerMotor.set(0);
        }
      else {
        drivetrain.arcadeDrive(0, 0);
        ROBOT_STATE = MANUAL;
        return;
      }
    }

    RSL::setState(RSL_ENABLED);
  } else {
    RSL::setState(RSL_DISABLED);
  }

  // No need to mess with this code
  RSL::update();
}
