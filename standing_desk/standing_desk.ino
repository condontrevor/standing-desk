#include "LogicData.h"
#include "buttons.h"
#include "button.h"

const uint8_t MOD_TX = 3;  // Pin 3 on Trinket supports interrupts
const uint8_t MOD_TX_interrupt  = 1;

const uint8_t INTF_TX = 9;

// Input/Output (v1.0 cable pins are arranged in this order)
const uint8_t MOD_HS2 = 4;
const uint8_t MOD_HS4 = 5;
const uint8_t MOD_HS1 = 6;
const uint8_t MOD_HS3 = 8;

const uint8_t MODE_LED = 7;
const uint8_t TIMER_LED = 9;
int TIMER_LED_STATE = LOW;


#define LED 13

unsigned long start_time;
unsigned long sit_time_seconds = 45*60;
unsigned long stand_time_seconds = 15*60;

//unsigned long sit_time_seconds = 1*60;
//unsigned long stand_time_seconds = 1*60;

unsigned long target_time = sit_time_seconds;
int WARNING_TIME_LEFT = 30;
enum {
  NEXT_STAND,
  NEXT_SIT
};
int next_state = NEXT_STAND;

enum {
  RANDOM_OFF,
  RANDOM_ON
};
int timer_state = RANDOM_OFF;

unsigned long last_signal = 0;
unsigned long last_latch = 0;

LogicData ld(INTF_TX);
Buttons btns(MOD_HS1, MOD_HS2, MOD_HS3, MOD_HS4);

enum {
  MODE_IDL,
  MODE_MOV,
  MODE_SET,
};
int mode = MODE_IDL;
int stand_height = 44;
int sit_height = 33;
int desired_height = sit_height;

unsigned gather_test = 0;
void dataGather() {
  digitalWrite(LED, digitalRead(MOD_TX));
  ld.PinChange(HIGH == digitalRead(MOD_TX));
  gather_test++;
}

Button random_timer_btn;

void setup() {
  pinMode(MOD_TX, INPUT);
  pinMode(MOD_HS1, INPUT);
  pinMode(MOD_HS2, INPUT);
  pinMode(MOD_HS3, INPUT);
  pinMode(MOD_HS4, INPUT);
  
  pinMode(TIMER_LED, OUTPUT);
  digitalWrite(TIMER_LED, LOW);
  pinMode(MODE_LED, OUTPUT);
  random_timer_btn.begin(10);

  dataGather();
  attachInterrupt(MOD_TX_interrupt, dataGather, CHANGE);

  ld.Begin();
  delay(1000);

  #ifdef LED
    pinMode(LED, OUTPUT);
  #endif
  Serial.begin(115200);
  Serial.println("Standing Desk v0.1 build: " __DATE__ " " __TIME__);
  start_time = millis();
}

unsigned int height = sit_height;

// Record last time the display changed
void check_display() {
  static uint32_t prev = 0;
  uint32_t msg = ld.ReadTrace();
  if (msg) {
    char buf[80];
    uint32_t now = millis();
    sprintf(buf, "%6lums %s: %s", now - prev, ld.MsgType(msg), ld.Decode(msg));
    Serial.println(buf);
    prev=now;
  }

  // Reset idle-activity timer if display number changes or if any other display activity occurs (i.e. display-ON)
  if (ld.IsNumber(msg)) {
    static uint8_t prev_number;
    auto display_num = ld.GetNumber(msg);
    height = display_num;
    if (display_num == prev_number) {
      return;
    }
    prev_number = display_num;
  }
  if (msg)
    last_signal = millis();
}


int command_data = 0;
int additional_buttons = 0;
void check_serial() {
  additional_buttons = 0;
  if(Serial.available() > 0){
    switch(Serial.read()){
      case 'u':
         Serial.println("simulated button push: UP");
         additional_buttons = UP;
         break;
      case 'd':
         Serial.println("simulated button push: DOWN");
         additional_buttons = DOWN;
         break;
      case '2':
         if (mode == MODE_SET) {
           if (height != 0) {
             stand_height = height;
             Serial.print("stand height set:");
             Serial.println(stand_height);
           }
           mode = MODE_IDL;
         }
         else {
           desired_height = stand_height;
           mode = MODE_MOV;
           Serial.print("Moving to: ");
           Serial.println(desired_height);
         }
         break;
      case '1':
         if (mode == MODE_SET) {
           if (height != 0) {
             sit_height = height;
             Serial.print("sit height set: ");
             Serial.println(sit_height);
           }
           mode = MODE_IDL;
         }
         else {
           desired_height = sit_height;
           mode = MODE_MOV;
           Serial.print("Moving to: ");
           Serial.println(desired_height);
         }
         break;
      case 's':
         Serial.println("SET");
         mode = MODE_SET;
         break;
    }
  }
}

void check_mode() {
  if (random_timer_btn.debounce()) {
     start_time = millis();
     TIMER_LED_STATE = LOW;
     Serial.println("Mode switch");
     if (timer_state == RANDOM_OFF) {
       timer_state = RANDOM_ON;
       digitalWrite(MODE_LED, HIGH);
     }
     else if (timer_state == RANDOM_ON) {
       timer_state = RANDOM_OFF;
       digitalWrite(MODE_LED, LOW);
     }
  }
}

int last_second = 0;
void check_timers() {
  unsigned long delta = millis() - start_time;
  unsigned long delta_seconds = delta/1000;
  if (delta_seconds != last_second) {
    last_second = delta_seconds;
    
    Serial.print("elapsed: ");
    Serial.print(delta_seconds);
    Serial.print("/");
    Serial.println(target_time);
    
    int remaining_seconds = target_time - delta_seconds;
    if (remaining_seconds < WARNING_TIME_LEFT) { 
      if (TIMER_LED_STATE == HIGH) { TIMER_LED_STATE = LOW; }
      else if (TIMER_LED_STATE == LOW) { TIMER_LED_STATE = HIGH; }
    }
    else {
      TIMER_LED_STATE = LOW;
    }

    if (delta_seconds > target_time) {
      start_time = millis();
      if (next_state == NEXT_SIT) {
        target_time = sit_time_seconds;
        desired_height = sit_height;
        mode = MODE_MOV;
        next_state = NEXT_STAND;
      }
      else if (next_state == NEXT_STAND) {
        target_time = stand_time_seconds;
        desired_height = stand_height;
        mode = MODE_MOV;
        next_state = NEXT_SIT;
      }
      Serial.print("Timer elapsed, moving to: ");
      Serial.println(desired_height);
    }
  }
}

int prev_height = 0;
void set_motors() {
  if (mode == MODE_MOV and height != 0) {
    if (height != prev_height) { 
      Serial.print("desired height: ");
      Serial.println(desired_height);
      Serial.print("current height: ");
      Serial.println(height);
    }
    if (height == desired_height) {
      mode = MODE_IDL;
      Serial.println("HEIGHT REACHED"); 
      pinMode(MOD_HS2, INPUT);
      pinMode(MOD_HS1, INPUT);
    }
    else if (height < desired_height) {
      pinMode(MOD_HS1, OUTPUT);
      digitalWrite(MOD_HS1, HIGH);
      pinMode(MOD_HS2, INPUT);
    }
    else if (height > desired_height) {
      pinMode(MOD_HS2, OUTPUT);
      digitalWrite(MOD_HS2, HIGH);
      pinMode(MOD_HS1, INPUT);
    }
    prev_height = height;
  }
}

void loop() {
  check_mode();
  check_display();
  check_serial();
  if (timer_state == RANDOM_ON) { check_timers(); }
  digitalWrite(TIMER_LED, TIMER_LED_STATE);
  set_motors();
}
