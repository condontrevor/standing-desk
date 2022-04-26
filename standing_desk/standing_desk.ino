
#define echoPin 2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3 //attach pin D3 Arduino to pin Trig of HC-SR04

enum desired_state {SIT, STAND};
enum motion_state {IDLE, MOTION};

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

const int numReadings = 20;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
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
  read_distance

  // check buttons, update state machine

  // check distance, update state machine

  // change state of motors based on state machine
  
}
