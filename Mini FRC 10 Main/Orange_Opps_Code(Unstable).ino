#include <PestoLink-Receive.h>
#include <Alfredo_NoU2.h>
#include <NewPing.h>

// Button definitions
#define BUTTON_BOTTOM 0
#define BUTTON_RIGHT 1
#define BUTTON_LEFT 2
#define BUTTON_TOP 3
#define LEFT_BUMPER 4
#define RIGHT_BUMPER 5
#define LEFT_TRIGGER 6
#define RIGHT_TRIGGER 7
#define R_PRESS 8
#define L_PRESS 9
#define D_UP 10
#define D_DOWN 11
#define D_LEFT 12
#define D_RIGHT 13
#define MID_RIGHT 14
#define MID_LEFT 15

// Motor and servo definitions
NoU_Motor frontLeftMotor(1); 
NoU_Motor frontRightMotor(2);
NoU_Motor rearLeftMotor(3);
NoU_Motor rearRightMotor(4);
NoU_Motor indexerMotor(5); // Intake/indexer motor
NoU_Motor shooterMotor(6);  // Shooter motor
NoU_Servo servo(1);         // Aiming servo

// Drivetrain setup
NoU_Drivetrain drivetrain(&frontLeftMotor, &frontRightMotor, &rearLeftMotor, &rearRightMotor);

// Distance sensor setup
#define TRIGGER_PIN 25
#define ECHO_PIN 26
#define MAX_DISTANCE 400
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Variables
float distance = 0;         // Distance measured to subwoofer
int measurements = 0;       // Number of measurements taken
int servoAngle = 90;        // Servo angle for aiming
float shooterThrottle = 0.1; // Shooter speed, needs tuning
const float SHOOTER_MIN_SPEED = 0.7; // Minimum speed where motors start working

// Time tracking for auto mode
long AUTO_START_TIME = 0;
long SHOOTER_START_TIME = 0;

// Enum for robot states (manual or auto mode)
enum State { MANUAL, AUTO };
State ROBOT_STATE;

// Auto mode constants
const int AUTO_MODE_DURATION = 14000; // Total duration of auto mode in ms

void setup() {
  PestoLink.begin("CallingAllBluds");  // Name your robot here
  Serial.begin(9600);
  
  // Invert motors if necessary
  frontLeftMotor.setInverted(true);
  frontRightMotor.setInverted(false);
  rearLeftMotor.setInverted(true);
  rearRightMotor.setInverted(false);

  // Initialize robot state to manual
  ROBOT_STATE = MANUAL;
}

void loop() {
  // Update connection status and handle robot states
  if (PestoLink.update()) {
    if (ROBOT_STATE == MANUAL) {
      handleManualMode();
    } else {
      handleAutoMode();
    }
    RSL::setState(RSL_ENABLED);
  } else {
    RSL::setState(RSL_DISABLED);
  }
  
  // No need to modify this
  RSL::update();
}

// Handle manual driving and shooting
void handleManualMode() {
  float indexerThrottle = 0;
  int AutoAngle;

  // Start auto mode if button is pressed
  if (PestoLink.buttonHeld(AUTO_START_BUTTON)) {
    ROBOT_STATE = AUTO;
    AUTO_START_TIME = millis();
    return;
  }

  // Driving controls
  float xVelocity = PestoLink.getAxis(0);
  float yVelocity = -PestoLink.getAxis(1);
  float rotation = PestoLink.getAxis(2);
  drivetrain.holonomicDrive(xVelocity, yVelocity, rotation);

  // Servo adjustment based on distance to subwoofer (when button pressed)
  if (PestoLink.buttonHeld(2)) {
    updateDistanceAndServo();
  }

  // Manual servo control
  manualServoControl();

  // Motor control for indexer and shooter
  handleIndexerAndShooter(indexerThrottle);
  
  // Apply motor throttle
  indexerMotor.set(indexerThrottle);
  shooterMotor.set(shooterThrottle);
}

// Handle servo control using distance sensor
// Handle servo control using distance sensor
void updateDistanceAndServo() {
  if (measurements < 50) {
    distance = sonar.ping() / 10; // Get distance in cm
    measurements++;
    Serial.println(distance);
    
    if (distance != 0.00) {
      // Tune distance offset as needed
      distance += 20; // Adjust this offset based on your setup (e.g., 20 cm)

      // Set servo angle based on distance using a regression function or lookup table
      servoAngle = calculateServoAngleFromDistance(distance);  // Use regression data here
      servo.write(servoAngle);
    }
  }
}

// Function to calculate servo angle from distance using regression model
int calculateServoAngleFromDistance(float distance) {
  // Placeholder for regression equation or data. 
  // You should replace this with your actual regression model or data points.

  // Example regression: Servo angle = a * distance^2 + b * distance + c
  // You need to replace a, b, c with the coefficients from your regression analysis.

  float a = -0.002;  // Example coefficient (change with your actual data)
  float b = 0.3;     // Example coefficient (change with your actual data)
  float c = 15;      // Example y-intercept (change with your actual data)

  // Calculate the servo angle using the regression formula
  int angle = a * (distance * distance) + b * distance + c;
  
  // Clamp the angle to a valid range for your servo
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;

  return angle;
}


// Manual servo control using joystick or buttons
void manualServoControl() {
  if (PestoLink.getRawAxis(3) > 230) {
    // Fine-tune servo angle if joystick pushed
    if (servo.getDegrees() > 0) {
      servo.write(servo.getDegrees() - 0.001);
    }
  }
}

// Handle the indexer and shooter motors
void handleIndexerAndShooter(float& indexerThrottle) {
  // Control the indexer (intake) motor
  if (PestoLink.buttonHeld(LEFT_BUMPER)) {
    indexerThrottle = 1;  // Intake forward
  } else if (PestoLink.buttonHeld(LEFT_TRIGGER)) {
    indexerThrottle = -1; // Intake reverse
  } else {
    indexerThrottle = 0;  // Stop intake
  }
  
  // Control the shooter motor
  if (PestoLink.buttonHeld(RIGHT_TRIGGER)) {
    shooterThrottle = SHOOTER_MIN_SPEED; // Spin shooter motor to shoot
    SHOOTER_START_TIME = millis();
  } else {
    shooterThrottle = 0;  // Stop shooter
  }

  // Timing for indexer based on shooting
  handleShooterTiming(indexerThrottle);
}

// Handle timing for shooter and indexer motors
void handleShooterTiming(float& indexerThrottle) {
  if (SHOOTER_START_TIME != 0) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - SHOOTER_START_TIME;

    if (elapsedTime > 1 && elapsedTime <= 1250) {
      indexerThrottle = 1;  // Run indexer during shoot
    } else if (elapsedTime > 1500) {
      SHOOTER_START_TIME = 0; // Reset after the shoot sequence
      measurements = 0;
    }
  }
}
float TSA = 0;
// Handle auto mode actions
void handleAutoMode() {
  if (PestoLink.buttonHeld(MID_RIGHT)) {
    ROBOT_STATE = MANUAL;
    return;
  }

  TSA = (millis() - AUTO_START_TIME);
    if(TSA < 14000){
      if(TSA < 1000){ //aim and spin up shooter
        servo.write(33);//subwoofer angle
        shooterMotor.set(-1);
      }
      if(TSA > 1000 && TSA < 1500){ //shoot
        indexerMotor.set(1);
      }
      if(TSA > 1500 && TSA < 2000){//reset motor
        resetMotors();
      }
      if(TSA > 2100 && TSA < 2200){//reset servo
        servo.write(0);
      }
      if(TSA > 2500 && TSA < 4000){//intake
        indexerMotor.set(1);
        if(TSA > 2600 && TSA < 3100/*change for distance*/){
          drivetrain.arcadeDrive(1, 0);
        } else{
          drivetrain.arcadeDrive(0, 0);
        }
      }
      if(TSA > 4000 && TSA < 4250){//reset motor
        indexerMotor.set(0);
      }
      if(TSA > 4000 && TSA < 4500/*change for distance*/){//drive to subwoofer
        drivetrain.arcadeDrive(-1, 0);
      }
      if(TSA > 4500 && TSA < 5600){//stop
        drivetrain.arcadeDrive(0,0);
      }
      if(TSA > 5600 && TSA < 7000){ //aim and spin up shooter
        drivetrain.arcadeDrive(0, 0);
        servo.write(33);//subwoofer angle
        shooterMotor.set(-1);
      }
      if(TSA > 7000 && TSA < 7500){ //shoot
        indexerMotor.set(1);
      }
      if(TSA > 7500 && TSA < 8000){//reset motor
        resetMotors();
      }
      if(TSA > 8000){//reset servo
        servo.write(0);
      }
    }
    else {
      drivetrain.arcadeDrive(0, 0);
      return;      
    }
}

// Reset motors (shooter and indexer)
void resetMotors() {
  shooterMotor.set(0);
  indexerMotor.set(0);
}
