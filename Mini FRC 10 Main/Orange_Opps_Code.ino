#include <PestoLink-Receive.h>
#include <Alfredo_NoU2.h>

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

// This lets us easily keep track of whether the robot is in auto mode or not.
// Experiment with adding more modes!
enum State { MANUAL, AUTO };

// These are the buttons we look for.
int AUTO_START_BUTTON = 2;
int AUTO_CANCEL_BUTTON = 9;

// This stores the time at which we started auto mode. This lets us keep track of how long we've been in auto mode for.
long AUTO_START_TIME = 0;
long SHOOTER_START_TIME = 0;
State ROBOT_STATE;

void setup() {
  //EVERYONE SHOULD CHANGE "ESP32 Bluetooth" TO THE NAME OF THEIR ROBOT
  PestoLink.begin("CallingAllBluds");
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

  //When PestoLink.update() is called, it returns "true" only if the robot is connected to phone/laptop  
  if (PestoLink.update()) {
    if (ROBOT_STATE == MANUAL) {
      // We only want to check manual controls if we're in manual mode!
      if (PestoLink.buttonHeld(2)) {
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
      float shootSpeed;
      if (PestoLink.buttonHeld(3)) {
        servoAngle = 140; //climber up angle/amp
        servo.write(servoAngle);
        shootSpeed = -0.75;
      } else if (PestoLink.buttonHeld(0)) {
        servoAngle = 29; //subwoofer angle
        servo.write(servoAngle);
        shootSpeed = -0.75;
      } else if (PestoLink.buttonHeld(1)) {
        servoAngle = 53; //podium
        servo.write(servoAngle);
      }
      
      // Motor Code:
      if (PestoLink.buttonHeld(4)) {
        indexerThrottle = 1;
        servo.write(-3);
      } else if (PestoLink.buttonHeld(6)) {
        indexerThrottle = -1;
        shooterThrottle = -0.75;
      } else {
        indexerThrottle = 0;
      }
      
      if (PestoLink.buttonHeld(5)) {
        shooterThrottle = 1;
      } else if (PestoLink.buttonHeld(7)) {
        shooterThrottle = -1;
        SHOOTER_START_TIME = millis();
      } else {
        if (PestoLink.buttonHeld(4)){
          shooterThrottle = 0.9;
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
      // If it's been less than one second (or, 1000 milliseconds) since we started auto mode, shoot.
      if ((millis() - AUTO_START_TIME) < 1000) {
        servo.write(33);
        shooterMotor.set(-1);
        SHOOTER_START_TIME = millis();
      } 
      // Otherwise, stop and exit auto mode.
      else if(((millis() - AUTO_START_TIME) > 999) && ((millis() - AUTO_START_TIME) < 2500)){
        if((millis() - AUTO_START_TIME) > 2200) {
          drivetrain.arcadeDrive(1, 0);
          indexerMotor.set(1);
        }
        indexerMotor.set(1);
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
