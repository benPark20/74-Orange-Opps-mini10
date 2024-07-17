#include <PestoLink-Receive.h>
#include <Alfredo_NoU2.h>

// If your robot has more than a drivetrain and one servo, add those actuators here 
NoU_Motor frontLeftMotor(5); //first port is broken as of now
NoU_Motor frontRightMotor(2);
NoU_Motor rearLeftMotor(3);
NoU_Motor rearRightMotor(4);
NoU_Servo servo(1);

// This creates the drivetrain object, you shouldn't have to mess with this
NoU_Drivetrain drivetrain(&frontLeftMotor, &frontRightMotor, &rearLeftMotor, &rearRightMotor);

// This lets us easily keep track of whether the robot is in auto mode or not.
// Experiment with adding more modes!
enum State { MANUAL, AUTO };

// These are the buttons we look for.
int AUTO_START_BUTTON = 8;
int AUTO_CANCEL_BUTTON = 9;

// This stores the time at which we started auto mode. This lets us keep track of how long we've been in auto mode for.
long AUTO_START_TIME = 0;
State ROBOT_STATE;

void setup() {
	//EVERYONE SHOULD CHANGE "ESP32 Bluetooth" TO THE NAME OF THEIR ROBOT
	PestoLink.begin("ESP32 Bluetooth");

	// If a motor in your drivetrain is spinning the wrong way, change the value for it here from 'false' to 'true'
    frontLeftMotor.setInverted(false);
    frontRightMotor.setInverted(false);
    rearLeftMotor.setInverted(false);
    rearRightMotor.setInverted(false);

	// No need to mess with this code
    RSL::initialize();
    RSL::setState(RSL_ENABLED);
    ROBOT_STATE = MANUAL;
}

void loop() {
	// Here we define the variables we use in the loop
    int throttle = 0;
    int rotation = 0;
    int servoAngle = 0;

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
          float xVelocity = PestoLink.getAxis(0) * 1.5;
          float yVelocity = -PestoLink.getAxis(1) * 1.5;
          float rotation = PestoLink.getAxis(2) * 1.5;
          
          drivetrain.holonomicDrive(xVelocity, yVelocity, rotation);
      
      // Here we decide what the servo angle will be based on if a button is pressed ()
      if (PestoLink.buttonHeld(0)) {
        servoAngle = 50;
      }
      else {
        servoAngle = 110;
      }
      
      // Here we set the drivetrain motor speeds and servo angle based on what we found in the above code
      servo.write(servoAngle);
      }
      else {
        // We're in auto mode, so we should handle auto mode.
        if (PestoLink.update()) {
          if (PestoLink.buttonHeld(AUTO_CANCEL_BUTTON)) {
            // Check to see if we should cancel auto mode.
            ROBOT_STATE = MANUAL;
            return;
          }
        }
        // If it's been less than one second (or, 1000 milliseconds) since we started auto mode, drive forwards.
        if ((millis() - AUTO_START_TIME) < 1000) {
          drivetrain.arcadeDrive(-1, 0);
        }
        // Otherwise, stop and exit auto mode.
        else {
          drivetrain.arcadeDrive(0,0);
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
