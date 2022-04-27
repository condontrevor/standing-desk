#include "lib/button.h"
#define echoPin 2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3 //attach pin D3 Arduino to pin Trig of HC-SR04

enum DesiredState {SIT, STAND};
enum MotionState {IDLED, MOVING};
enum MotorDirection {UP, DOWN};

DesiredState desired = SIT;
MotionState motion = IDLED;
MotorDirection motor_direction = UP;

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

Button button_1;
Button button_2;

const int numReadings = 20;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

const int THRESHOLD = 10; // cm
int sitting_height = 30; // ??
int standing_height = 60; // ??


void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
  button_1.begin(4);
  button_2.begin(5);
}

// Running average
void read_distance() {
  // Measure distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  // Update measurement array
  total = total - readings[readIndex];
  readings[readIndex] = duration * 0.034 / 2;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {readIndex = 0;}

  // Calculate the average
  distance = total / numReadings;
}

void loop() {
  read_distance();

  // check buttons, update state machine
  if (button_1.debounce()) {
    digitalWrite(LED_BUILTIN, HIGH);
    if (desired == SIT) {
      desired = STAND;
      motion = MOVING;
    }
  }
  if (button_2.debounce()) {
    digitalWrite(LED_BUILTIN, LOW);
    if (desired == STAND) {
      desired = SIT;
      motion = MOVING;
    }
  }
 
  // check distance, update state machine
  if (desired == STAND && motion == MOVING) {
    if (abs(standing_height - distance) < THRESHOLD) {
      motion = IDLED;
    }
    else {
      if (standing_height > distance) {
        motor_direction = UP;
      }
      else {
        motor_direction = DOWN;
      }
    }
  }

  // check distance, update state machine
  if (desired == SIT && motion == MOVING) {
    if (abs(sitting_height - distance) < THRESHOLD) {
      motion = IDLED;
    }
    else {
      if (standing_height > distance) {
        motor_direction = UP;
      }
      else {
        motor_direction = DOWN;
      }
    }
  }

  // change state of motors based on state machine
  if (motion == MOVING) {
    if (motor_direction == UP) {
      // CLEAR THE MOTOR DOWN PIN
      // SET THE MOTOR UP PIN
    }
    else {
      // CLEAR THE MOTOR UP PIN
      // SET THE MOTOR DOWN PIN
    }
  }
  else {
    // CLEAR THE MOTOR UP PIN
    // CLEAR THE MOTOR DOWN PIN
  }
  
}
