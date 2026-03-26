#include <Arduino.h>

// ==========================
// L298N pin definitions
// Change these to match your wiring
// ==========================

// Motor 1 (left front)
#define M1_IN1 22
#define M1_IN2 23
#define M1_PWM 2

// Motor 2 (right front)
#define M2_IN1 24
#define M2_IN2 25
#define M2_PWM 3

// Motor 3 (left rear)
#define M3_IN1 26
#define M3_IN2 27
#define M3_PWM 4

// Motor 4 (right rear)
#define M4_IN1 28
#define M4_IN2 29
#define M4_PWM 5

// ==========================
// Optional motor inversion
// Set true if a motor spins opposite of what you expect
// ==========================
bool invertM1 = false;
bool invertM2 = false;
bool invertM3 = false;
bool invertM4 = false;

void setup() {
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M1_PWM, OUTPUT);

  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);
  pinMode(M2_PWM, OUTPUT);

  pinMode(M3_IN1, OUTPUT);
  pinMode(M3_IN2, OUTPUT);
  pinMode(M3_PWM, OUTPUT);

  pinMode(M4_IN1, OUTPUT);
  pinMode(M4_IN2, OUTPUT);
  pinMode(M4_PWM, OUTPUT);

  Serial.begin(9600);
  stopMotors();
}

// ==========================
// Set one motor speed
// speed: -255 to +255
// positive = forward
// negative = reverse
// ==========================
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

// ==========================
// Set left and right sides
// leftSpeed/rightSpeed: -255 to +255
// ==========================
void setDrive(int leftSpeed, int rightSpeed) {
  setMotor(M1_IN1, M1_IN2, M1_PWM, leftSpeed, invertM1);
  setMotor(M3_IN1, M3_IN2, M3_PWM, leftSpeed, invertM3);

  setMotor(M2_IN1, M2_IN2, M2_PWM, rightSpeed, invertM2);
  setMotor(M4_IN1, M4_IN2, M4_PWM, rightSpeed, invertM4);
}

void stopMotors() {
  setDrive(0, 0);
}

void loop() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      input.trim();

      // ==========================
      // Expected format:
      // D,leftSpeed,rightSpeed
      // Example: D,200,120
      // ==========================
      if (input.startsWith("D,")) {
        int firstComma = input.indexOf(',');
        int secondComma = input.indexOf(',', firstComma + 1);

        if (firstComma > 0 && secondComma > firstComma) {
          int leftSpeed = input.substring(firstComma + 1, secondComma).toInt();
          int rightSpeed = input.substring(secondComma + 1).toInt();

          leftSpeed = constrain(leftSpeed, -255, 255);
          rightSpeed = constrain(rightSpeed, -255, 255);

          setDrive(leftSpeed, rightSpeed);
        }
      }
      else if (input == "S") {
        stopMotors();
      }

      input = "";
    }
    else {
      input += c;
    }
  }
}