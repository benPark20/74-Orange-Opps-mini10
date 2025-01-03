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
#define MID_RIGHT 8
#define MID_LEFT 9
#define L_PRESS 10
#define R_PRESS 11
#define D_UP 12
#define D_DOWN 13
#define D_LEFT 14
#define D_RIGHT 15

// Motor and servo definitions
NoU_Motor frontLeftMotor(1); 
NoU_Motor frontRightMotor(2);
NoU_Motor rearLeftMotor(3);
NoU_Motor rearRightMotor(4);
NoU_Motor indexerMotor(5); // Intake/indexer motor
NoU_Motor shooterMotor(6); // Shooter motor
NoU_Servo servo(1);        // Aiming servo

// Drivetrain setup
NoU_Drivetrain drivetrain(&frontLeftMotor, &frontRightMotor, &rearLeftMotor, &rearRightMotor);

// Distance sensor setup
#define TRIGGER_PIN 25
#define ECHO_PIN 26
#define MAX_DISTANCE 400
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Variables
int distance = 0;         // Distance measured to subwoofer
int measurements = 0;       // Number of measurements taken
int servoAngle = 0;         // Servo angle for aiming
float indexerThrottle = 0;
float shooterThrottle = 0;

// Time tracking for auto mode
long AUTO_START_TIME = 0;
long SHOOTER_START_TIME = 0;

// Enum for robot states (manual or auto mode)
enum State { MANUAL, AUTO };
State ROBOT_STATE;

void setup() {
  PestoLink.begin("Clementiny");  // Name your robot here
  Serial.begin(9600);
  Serial.println("Start");
  
  // Invert motors if necessary
  frontLeftMotor.setInverted(true);
  frontRightMotor.setInverted(false);
  rearLeftMotor.setInverted(true);
  rearRightMotor.setInverted(false);
  shooterMotor.setInverted(true);
  indexerMotor.setInverted(false);

  // Initialize robot state to manual
  ROBOT_STATE = MANUAL;
}

void loop() {
  // Update connection status and handle robot states
  if (PestoLink.update()) {
    if (ROBOT_STATE == MANUAL) {
      handleManualMode();
      PestoLink.print("Teleop", "0x32a852");
    } else {
      handleAutoMode();
      PestoLink.print("Auto", "0x36a0d1" );
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

  // Start auto mode if button is pressed
  if (PestoLink.buttonHeld(MID_LEFT)) {
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
  if (PestoLink.buttonHeld(BUTTON_LEFT)) {
    updateDistanceAndServo();
  }

  // Manual servo control
  manualServoControl();

  // Motor control for indexer and shooter
  handleIndexerAndShooter();
  
  // Apply motor throttle
  indexerMotor.set(indexerThrottle);
  shooterMotor.set(shooterThrottle);

  // Apply Servo Angle
  servo.write(servoAngle);
}

// Handle servo control using distance sensor
void updateDistanceAndServo() {
  if (measurements < 50) {
    int distance = sonar.ping_cm();// Get distance in cm
    int trash = sonar.ping_cm();
    measurements++;
    
    if (distance != 0.00) {
      // Tune distance offset as needed
      distance += -2;
      if(distance <= 0){distance = 0;}

      // Set servo angle based on distance using a regression function or lookup table
      servoAngle = calculateServoAngleFromDistance(distance);  // Use regression data here
      char buffer[64];
      sprintf(buffer, "Distance: %i || Angle: %i", distance, servoAngle);
      PestoLink.printToTerminal(buffer);
    }
  }
}

// Function to calculate servo angle from distance using regression model
int calculateServoAngleFromDistance(float distance) {

  int x = distance;
  float a = -7705.458;
  float b = 0.97141;
  float c = -104.09;
  float d = 97.8772;

  float angle = (a * (b / (x - c)) + d);

  float A = 1.13565;
  float B = 0.302267;
  float C = -1.58807;
  float D = -0.128382;

  angle += (A * cos(B * x + C) + D);
  
  // Clamp the angle to a valid range for your servo
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;

  return angle;
}


// Manual servo control using joystick or buttons'
int lastA = -1;

void manualServoControl() {
  if (PestoLink.buttonHeld(D_DOWN) && servoAngle > 0) {
    servoAngle -= 1;
    delay(100);
    char buffer[64];
    sprintf(buffer, "Angle: %i", servoAngle);
    PestoLink.printToTerminal(buffer);
  }
  if (PestoLink.buttonHeld(D_UP) && servoAngle < 180){
    servoAngle += 1;
    delay(100);
    char buffer[64];
    sprintf(buffer, "Angle: %i", servoAngle);
    PestoLink.printToTerminal(buffer);
  }
  if(PestoLink.buttonHeld(BUTTON_BOTTOM)){
    servoAngle = 120;//amp
    PestoLink.printToTerminal("Amp Angle");
  }
  else if(PestoLink.buttonHeld(BUTTON_RIGHT)){
    servoAngle = 80;//pass
    PestoLink.printToTerminal("Passing Angle");
  }
  else if(PestoLink.buttonHeld(R_PRESS)){
    updateDistanceAndServo();
  }

  if(servoAngle != lastA){
    lastA = servoAngle;
  }
}

// Handle the indexer and shooter motors
void handleIndexerAndShooter() {
  // Control the indexer (intake) motor
  if (PestoLink.buttonHeld(LEFT_BUMPER)) {
    indexerThrottle = 1;  // Intake forward
    servoAngle = 0;
    PestoLink.printToTerminal("Intaking. . .");
  } else if (PestoLink.buttonHeld(LEFT_TRIGGER)) {
    indexerThrottle = -1; // Intake reverse
  } else {
    indexerThrottle = 0;  // Stop intake
  }
  
  // Control the shooter motor
  if (PestoLink.buttonHeld(RIGHT_TRIGGER)) {
    shooterThrottle = 1; // Spin shooter motor to shoot
    SHOOTER_START_TIME = millis();
    updateDistanceAndServo();
  } else if(PestoLink.buttonHeld(RIGHT_BUMPER)){
    servoAngle = 25;
    shooterThrottle = 1;
    SHOOTER_START_TIME = millis();
  } else if(PestoLink.buttonHeld(BUTTON_TOP)){
    shooterThrottle = 1;
    SHOOTER_START_TIME = millis();
  } else if(PestoLink.buttonHeld(LEFT_BUMPER) || PestoLink.buttonHeld(LEFT_TRIGGER)){
    shooterThrottle = -1;
  } else {
    shooterThrottle = 0;
  }

  // Reset measurments
  if(PestoLink.buttonHeld(BUTTON_LEFT)){
    measurements = 0;
  }

  // Timing for indexer based on shooting
  handleShooterTiming();
}

// Handle timing for shooter and indexer motors
void handleShooterTiming() {
  if (SHOOTER_START_TIME != 0 ) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - SHOOTER_START_TIME;

    if (elapsedTime > 100 && elapsedTime <= 1200) {
      indexerThrottle = 1;  // Run indexer during shoot
      shooterThrottle = 0.7;
      PestoLink.printToTerminal("Shots Fired!");
    } else if (elapsedTime > 1200 && elapsedTime < 1400) {
      measurements = 0;
      shooterThrottle = -1;
    } else if (elapsedTime >= 1400) {
      resetMotors();
      SHOOTER_START_TIME = 0; // Reset after the shoot sequence
      return;
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
        shooterMotor.set(1);
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
        shooterMotor.set(-1);
        if(TSA > 2600 && TSA < 3100/*change for distance*/){
          drivetrain.arcadeDrive(1, 0);
        } else{
          drivetrain.arcadeDrive(0, 0);
        }
      }
      if(TSA > 4000 && TSA < 4250){//reset motor
        resetMotors();
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
        shooterMotor.set(1);
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
