/* ---------------------------------------------------------------------------
  ~ Gradient Square Generator ~

    Creates a color gradient from left to right, this time with squares!
    Right colors set using RGB, controlled by three potentiometers on board
    Right colors copied to left by pressing button on board

  --------------------------------------------------------------------------- */

#include <WS2812Serial.h>
#define num_of_leds 64
#define led_data_pin 5
byte drawingMemory[num_of_leds * 3];
DMAMEM byte displayMemory[num_of_leds * 12];
WS2812Serial leds(num_of_leds, displayMemory, drawingMemory, led_data_pin, WS2812_GRB);
float max_brightness = 0.1;

#define left_button_pin 0
#define top_left_pot_pin A0
#define top_right_pot_pin A1
#define bottom_left_pot_pin A2
#define bottom_right_pot_pin A3

unsigned long current_time;
unsigned long prev_time[8];

int xy_count;
int button_state;

// framerate
int rate1 = 30;

// rate at which rbg values are logged, in ms
int logging_rate = 250;

// starting left rbg values, right set by pot values
int left_r = 255;
int left_g = 0;
int left_b = 66;
int right_r;
int right_g;
int right_b;

void setup() {
  leds.begin(); // must be done in setup for the LEDs to work.
  pinMode(left_button_pin, INPUT_PULLUP); // must be done when reading buttons
  analogReadResolution(12); // 0-4095 pot values
  analogReadAveraging(64);  // smooth the readings some
}

void loop() {
  current_time = millis();

  if (current_time - prev_time[0] > rate1) {
    prev_time[0] = current_time;

    // button to copy right to left
    button_state = digitalRead(left_button_pin);

    // reads RGB values for right LEDs from pots
    right_r = analogRead(bottom_left_pot_pin) / 16;
    right_g = analogRead(top_left_pot_pin) / 16;
    right_b = analogRead(bottom_right_pot_pin) / 16;

    // slow logging function bc we don't need that every 30 ms
    // couldn't find a way to cast int to str so here we are
    if (current_time - prev_time[1] > logging_rate) {
      prev_time[1] = current_time;

      Serial.print("Right RGB: (");
      Serial.print(right_r);
      Serial.print(",");
      Serial.print(right_g);
      Serial.print(",");
      Serial.print(right_b);
      Serial.println(")");
      Serial.print("Left RGB: (");
      Serial.print(left_r);
      Serial.print(",");
      Serial.print(left_g);
      Serial.print(",");
      Serial.print(left_b);
      Serial.println(")");
    }

    // sweep through LEDs and set color gradient between left and right columns
    for (int i = 0; i < 64; i++) {
      leds.setPixelColor(i,
                         left_r + i * (right_r - left_r) / 63,
                         left_g + i * (right_g - left_g) / 63,
                         left_b + i * (right_b - left_b) / 63);
    }


    // when button is pressed, copy right LED color to left column
    if (button_state == 0) {
      left_r = right_r;
      left_g = right_g;
      left_b = right_b;
    }
    leds.show();
  }
}
