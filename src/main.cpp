#include <Arduino.h>
#include <Servo.h>

#define DIR_M1 22
#define PWM_M1 2
#define DIR_M2 23
#define PWM_M2 3
#define DIR_M3 24
#define PWM_M3 4
#define DIR_M4 25
#define PWM_M4 5

void setup() {
  pinMode(DIR_M1, OUTPUT);
  pinMode(PWM_M1, OUTPUT);
  pinMode(DIR_M2, OUTPUT);
  pinMode(PWM_M2, OUTPUT);
  pinMode(DIR_M3, OUTPUT);
  pinMode(PWM_M3, OUTPUT);
  pinMode(DIR_M4, OUTPUT);
  pinMode(PWM_M4, OUTPUT);

  Serial.begin(9600);
}

void setMotors(bool dir, int speed) {
  digitalWrite(DIR_M1, dir);
  digitalWrite(DIR_M2, dir);
  digitalWrite(DIR_M3, dir);
  digitalWrite(DIR_M4, dir);

  analogWrite(PWM_M1, speed);
  analogWrite(PWM_M2, speed);
  analogWrite(PWM_M3, speed);
  analogWrite(PWM_M4, speed);
}

void stopMotors() {
  analogWrite(PWM_M1, 0);
  analogWrite(PWM_M2, 0);
  analogWrite(PWM_M3, 0);
  analogWrite(PWM_M4, 0);
}

void loop() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      // Parse message format: DIRECTION,SPEED
      int commaIndex = input.indexOf(',');
      if (commaIndex > 0) {
        char cmd = input.charAt(0);
        int speed = input.substring(commaIndex + 1).toInt();
        speed = constrain(speed, 0, 255);

        if (cmd == 'L') setMotors(LOW, speed);
        else if (cmd == 'R') setMotors(HIGH, speed);
        else if (cmd == 'B') { // turn left
          digitalWrite(DIR_M1, HIGH);
          digitalWrite(DIR_M2, LOW);
          digitalWrite(DIR_M3, HIGH);
          digitalWrite(DIR_M4, LOW);
          analogWrite(PWM_M1, speed);
          analogWrite(PWM_M2, speed);
          analogWrite(PWM_M3, speed);
          analogWrite(PWM_M4, speed);
        }
        else if (cmd == 'F') { // turn right
          digitalWrite(DIR_M1, LOW);
          digitalWrite(DIR_M2, HIGH);
          digitalWrite(DIR_M3, LOW);
          digitalWrite(DIR_M4, HIGH);
          analogWrite(PWM_M1, speed);
          analogWrite(PWM_M2, speed);
          analogWrite(PWM_M3, speed);
          analogWrite(PWM_M4, speed);
        }
        else if (cmd == 'S') stopMotors();
      }
      input = "";
    } else {
      input += c;
    }
  }
}
