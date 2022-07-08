#ifndef buttons_h
#define buttons_h

#include "Arduino.h"

enum {
  HS1 = 1,
  HS2 = 2,
  HS3 = 4,
  HS4 = 8
};

#define UP    HS1
#define DOWN  HS2
#define SET   (HS1 + HS2)
#define MEM1  HS3
#define MEM2  HS4
#define MEM3  (HS2 + HS4)
#define NONE  0

class Buttons
{
  private:
    unsigned latched;
    unsigned long latch_time;
    unsigned long last_action;
    bool draining;

    uint8_t hs_1;
    uint8_t hs_2;
    uint8_t hs_3;
    uint8_t hs_4;
  public:
    Buttons(uint8_t hs1, uint8_t hs2, uint8_t hs3, uint8_t hs4);
    void display_buttons(unsigned buttons, const char * msg = "");
    unsigned read_buttons();
    unsigned read_buttons_debounce();
    void clear_buttons();

    void set_latch(unsigned latch_pins, unsigned long max_latch_time = 15000);
    void unlatch();
    void break_latch();
    bool is_latched();
    unsigned get_latched();
    void latch_pin(int pin);
    void apply_latch();
};

#endif