#include <NewPing.h>

/*
 * Logic:
 *    1. read distance from ground
 *    2. read buttons, determine direction of travel
 *    3. if direction of travel changed from previous, set motion direction
 *    4. if motion direction, set motors appropriately
 *    5. once distance threshold is hit set motion to idle and stop motor
 *    
 */

#include "lib/button.h"
#define ECHO_PIN 2
#define TRIGGER_PIN 3 
#define MAX_DISTANCE 200

#define BUTTON_1_PIN 4
#define BUTTON_2_PIN 5

#define DEBUG
#define DEBUG_LED_1 6
#define DEBUG_LED_2 7

enum DesiredState {SIT, STAND};
enum MotorDirection {STOP, UP, DOWN};

DesiredState desired = SIT;
MotorDirection motor_direction = STOP;

Button button_1;
Button button_2;

unsigned long distance;
const int num_readings = 20;
unsigned long readings[num_readings];
int readIndex = 0;
unsigned long total = 0;
unsigned long average = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

const int THRESHOLD = 3;
const int DISTANCE_DELTA_ALLOWED = 60;
int sitting_height = 86;
int standing_height = 120;
int desired_height = sitting_height;

void setup() {
  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);
#ifdef DEBUG
  pinMode(DEBUG_LED_1, OUTPUT); 
  pinMode(DEBUG_LED_2, OUTPUT);
#endif
  Serial.begin(115200); // // Serial Communication is starting with 9600 of baudrate speed
  button_1.begin(BUTTON_1_PIN);
  button_2.begin(BUTTON_2_PIN);
  Serial.println("STANDING DESK!");
}

bool good_reading = false;
// Running average
void read_distance() {
  // Throw away bad measurements, calcuate the current average 
  distance = sonar.ping_cm();
  if (distance == 0) {
    good_reading = false;
    return;
  }
  good_reading = true;
  total = total - readings[readIndex];
  readings[readIndex] = distance;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= num_readings) {readIndex = 0;}
  distance = total / num_readings;
}

void loop() {
  read_distance();
  if (!good_reading) {
    motor_direction = STOP;
    digitalWrite(debugLED1, LOW);
    digitalWrite(debugLED2, LOW);
    Serial.println("bad reading");
    return;
  }
  
  // check buttons, update state machine
  if (button_2.debounce()) {
    Serial.println("button pushed");
    if (desired == STAND) {
      desired = SIT;
      motor_direction = DOWN;
      desired_height = sitting_height;
    }
    else {
      desired = STAND;
      motor_direction = UP;
      desired_height = standing_height;
    }
  }
  Serial.print("distance: ");
  Serial.print(distance);
  Serial.print(" desired: ");
  Serial.print(desired_height);
  Serial.print(" motor_direction:");
  Serial.println(motor_direction);

  // check distance, update state machine
  if (motor_direction == UP || motor_direction == DOWN) {
    if (abs(desired_height - distance) < THRESHOLD) {
      motor_direction = STOP;
      digitalWrite(debugLED1, LOW);
      digitalWrite(debugLED2, LOW);
    }
    else {
      if (desired_height > distance) {
        digitalWrite(debugLED1, LOW);
        digitalWrite(debugLED2, HIGH);
      }
      else {
        digitalWrite(debugLED1, HIGH);
        digitalWrite(debugLED2, LOW);
      }
    }
  }
//  delay(10);
}
