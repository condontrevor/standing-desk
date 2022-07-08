#include "buttons.h"

/** Interface to Robodesk handset buttons
 */
Buttons::Buttons(uint8_t hs1, uint8_t hs2, uint8_t hs3, uint8_t hs4) {
  hs_1 = hs1;
  hs_2 = hs2;
  hs_3 = hs3;
  hs_4 = hs4;

  latched = 0;
  latch_time = 0;
  last_action = 0;
  draining = false;
}

// Display anything that changed since last time
void Buttons::display_buttons(unsigned buttons, const char * msg ) {
  static unsigned prev = 0;

  if (msg[0] || prev!=buttons) {
    Serial.print((buttons & HS1) ? " HS1" :  " ---");
    Serial.print((buttons & HS2) ? " HS2" :  " ---");
    Serial.print((buttons & HS3) ? " HS3" :  " ---");
    Serial.print((buttons & HS4) ? " HS4" :  " ---");

    switch (buttons) {
      case UP:    Serial.print("    UP          ");    break;
      case DOWN:  Serial.print("    DOWN        ");    break;
      case SET:   Serial.print("    SET         ");    break;
      case MEM1:  Serial.print("    MEM1        ");    break;
      case MEM2:  Serial.print("    MEM2        ");    break;
      case MEM3:  Serial.print("    MEM3        ");    break;
      case NONE:  Serial.print("    ----        ");    break;
      default:    Serial.print(" ** UNKNOWN **  ");    break;
    }
    Serial.println(msg);
  }
  prev = buttons;
}

bool Buttons::is_latched() {
  return !!latched;
}

unsigned Buttons::get_latched() {
  return latched;
}

void Buttons::unlatch() {
  pinMode(hs_1, INPUT);
  pinMode(hs_2, INPUT);
  pinMode(hs_3, INPUT);
  pinMode(hs_4, INPUT);
}

void Buttons::latch_pin(int pin)
{
#ifndef DISABLE_LATCHING
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
#endif
}

void Buttons::break_latch() {
  latched = 0;
  unlatch();
}

void Buttons::apply_latch() {
  if (!latched) return;

  unsigned long delta = millis() - last_action;

  // Let go if latch timer expires
  if (delta > latch_time) {
    display_buttons(latched, "Timed out");
    break_latch();
    return;
  }

  // Re-assert latched buttons
  if (latched & HS1) latch_pin(hs_1);
  if (latched & HS2) latch_pin(hs_2);
  if (latched & HS3) latch_pin(hs_3);
  if (latched & HS4) latch_pin(hs_4);
}

void Buttons::set_latch(unsigned latch_pins, unsigned long max_latch_time) {
  if (latched == latch_pins) {
    if (latch_time + max_latch_time > latch_time) {
      latch_time += max_latch_time;
    }
  } else {
    latched = latch_pins;
    latch_time = max_latch_time;
  }

  // Restart idle timer when we get an interesting button
  last_action = millis();

  // Activate the selected latch pins
  apply_latch();

  char buf[50];
  sprintf(buf, "Latched up to %lums", latch_time);
  display_buttons(latched, buf);
}

unsigned Buttons::read_buttons() {
  unsigned buttons = 0;

  unlatch();
  if (digitalRead(hs_1)) buttons |= HS1;
  if (digitalRead(hs_2)) buttons |= HS2;
  if (digitalRead(hs_3)) buttons |= HS3;
  if (digitalRead(hs_4)) buttons |= HS4;
  apply_latch();
  
  return buttons;
}

void Buttons::clear_buttons() {
  draining = true;
}

unsigned Buttons::read_buttons_debounce() {
  static unsigned long debounce = 0;
  static unsigned prev_buttons = 0;

  unsigned diff = prev_buttons;
  prev_buttons = read_buttons();

  // Wait for interface to drain
  if (draining and prev_buttons) return NONE;
  draining = false;
  
  unsigned buttons = prev_buttons;

  // Ignore spurious signals
  if (diff && diff != prev_buttons) buttons = NONE;

  // Reset timer if buttons are drifting or unpressed
  if (!buttons) debounce = millis();

  // ignore signal until stable for 1-2ms
  if (millis()-debounce < 2) {
    buttons = NONE;
  }

  display_buttons(buttons);
  
  return buttons;
}
