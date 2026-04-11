#include <Arduino.h>
#include <Servo.h>

// =====================================
// IAM3D Excavator Robot - Arduino Mega
// 4 drivetrain motors
// 3 linear actuators
// 1 base swivel servo
// =====================================

// -----------------------------
// DRIVETRAIN MOTOR CHANNELS
// 2 L298Ns total for drivetrain
// left front, left rear, right front, right rear
// -----------------------------
#define LF_IN1   22
#define LF_IN2   23
#define LF_PWM   5

#define LR_IN1   24
#define LR_IN2   25
#define LR_PWM   6

#define RF_IN1   26
#define RF_IN2   27
#define RF_PWM   7

#define RR_IN1   28
#define RR_IN2   29
#define RR_PWM   8

// -----------------------------
// LINEAR ACTUATOR L298Ns
// -----------------------------
#define BOOM_IN1    30
#define BOOM_IN2    31
#define BOOM_PWM    9

#define DIPPER_IN1  32
#define DIPPER_IN2  33
#define DIPPER_PWM  10

#define BUCKET_IN1  34
#define BUCKET_IN2  35
#define BUCKET_PWM  11

// -----------------------------
// BASE SWIVEL SERVO
// -----------------------------
#define BASE_SERVO_PIN 12
Servo baseServo;

// -----------------------------
// Servo state
// -----------------------------
int basePos = 90;
const int BASE_MIN = 20;
const int BASE_MAX = 160;
const int BASE_STEP = 3;

// -----------------------------
// Optional inversion flags
// -----------------------------
bool invertLF     = false;
bool invertLR     = false;
bool invertRF     = true;
bool invertRR     = true;

bool invertBoom   = false;
bool invertDipper = false;
bool invertBucket = false;

// -----------------------------
// Generic motor helper
// speed: -255 to 255
// -----------------------------
void setMotor(int in1, int in2, int pwmPin, int speed, bool invertDirection = false) {
  speed = constrain(speed, -255, 255);

  if (invertDirection) {
    speed = -speed;
  }

  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(pwmPin, speed);
  }
  else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(pwmPin, -speed);
  }
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(pwmPin, 0);
  }
}

void setLeftDrive(int speed) {
  setMotor(LF_IN1, LF_IN2, LF_PWM, speed, invertLF);
  setMotor(LR_IN1, LR_IN2, LR_PWM, speed, invertLR);
}

void setRightDrive(int speed) {
  setMotor(RF_IN1, RF_IN2, RF_PWM, speed, invertRF);
  setMotor(RR_IN1, RR_IN2, RR_PWM, speed, invertRR);
}

void stopDrive() {
  setLeftDrive(0);
  setRightDrive(0);
}

void stopActuators() {
  setMotor(BOOM_IN1, BOOM_IN2, BOOM_PWM, 0, invertBoom);
  setMotor(DIPPER_IN1, DIPPER_IN2, DIPPER_PWM, 0, invertDipper);
  setMotor(BUCKET_IN1, BUCKET_IN2, BUCKET_PWM, 0, invertBucket);
}

void stopAll() {
  stopDrive();
  stopActuators();
}

void setup() {
  pinMode(LF_IN1, OUTPUT);
  pinMode(LF_IN2, OUTPUT);
  pinMode(LF_PWM, OUTPUT);

  pinMode(LR_IN1, OUTPUT);
  pinMode(LR_IN2, OUTPUT);
  pinMode(LR_PWM, OUTPUT);

  pinMode(RF_IN1, OUTPUT);
  pinMode(RF_IN2, OUTPUT);
  pinMode(RF_PWM, OUTPUT);

  pinMode(RR_IN1, OUTPUT);
  pinMode(RR_IN2, OUTPUT);
  pinMode(RR_PWM, OUTPUT);

  pinMode(BOOM_IN1, OUTPUT);
  pinMode(BOOM_IN2, OUTPUT);
  pinMode(BOOM_PWM, OUTPUT);

  pinMode(DIPPER_IN1, OUTPUT);
  pinMode(DIPPER_IN2, OUTPUT);
  pinMode(DIPPER_PWM, OUTPUT);

  pinMode(BUCKET_IN1, OUTPUT);
  pinMode(BUCKET_IN2, OUTPUT);
  pinMode(BUCKET_PWM, OUTPUT);

  baseServo.attach(BASE_SERVO_PIN);
  baseServo.write(basePos);

  Serial.begin(9600);
  stopAll();
}

void loop() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      input.trim();

      // --------------------------
      // DRIVE,left,right
      // Example: DRIVE,120,-200
      // --------------------------
      if (input.startsWith("DRIVE,")) {
        int firstComma = input.indexOf(',');
        int secondComma = input.indexOf(',', firstComma + 1);

        if (firstComma > 0 && secondComma > firstComma) {
          int leftSpeed = input.substring(firstComma + 1, secondComma).toInt();
          int rightSpeed = input.substring(secondComma + 1).toInt();

          leftSpeed = constrain(leftSpeed, -255, 255);
          rightSpeed = constrain(rightSpeed, -255, 255);

          setLeftDrive(leftSpeed);
          setRightDrive(rightSpeed);
        }
      }

      // --------------------------
      // BOOM,speed
      // --------------------------
      else if (input.startsWith("BOOM,")) {
        int comma = input.indexOf(',');
        if (comma > 0) {
          int speed = input.substring(comma + 1).toInt();
          speed = constrain(speed, -255, 255);
          setMotor(BOOM_IN1, BOOM_IN2, BOOM_PWM, speed, invertBoom);
        }
      }

      // --------------------------
      // DIPPER,speed
      // --------------------------
      else if (input.startsWith("DIPPER,")) {
        int comma = input.indexOf(',');
        if (comma > 0) {
          int speed = input.substring(comma + 1).toInt();
          speed = constrain(speed, -255, 255);
          setMotor(DIPPER_IN1, DIPPER_IN2, DIPPER_PWM, speed, invertDipper);
        }
      }

      // --------------------------
      // BUCKET,speed
      // --------------------------
      else if (input.startsWith("BUCKET,")) {
        int comma = input.indexOf(',');
        if (comma > 0) {
          int speed = input.substring(comma + 1).toInt();
          speed = constrain(speed, -255, 255);
          setMotor(BUCKET_IN1, BUCKET_IN2, BUCKET_PWM, speed, invertBucket);
        }
      }

      // --------------------------
      // BASE,dir
      // dir = -1, 0, 1
      // --------------------------
      else if (input.startsWith("BASE,")) {
        int comma = input.indexOf(',');
        if (comma > 0) {
          int dir = input.substring(comma + 1).toInt();

          if (dir == -1) basePos -= BASE_STEP;
          else if (dir == 1) basePos += BASE_STEP;

          basePos = constrain(basePos, BASE_MIN, BASE_MAX);
          baseServo.write(basePos);
        }
      }

      else if (input == "STOP_ALL") {
        stopAll();
      }

      input = "";
    }
    else {
      input += c;
    }
  }
}