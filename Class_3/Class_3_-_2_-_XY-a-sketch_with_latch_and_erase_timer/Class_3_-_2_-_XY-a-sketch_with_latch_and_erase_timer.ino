// Drawing on the grid


//This first block is all copy-paste and can be left alone except for brightness it just sets up the library

//led biz begin
#include <WS2812Serial.h>
//we'll be using the Teensy audio library and it doesn't play nicely with neopixels.h or fastled
// so Paul of PJRC made this much more efficient version
const int num_of_leds = 64;
const int pin = 5; // only these pins can be used on the Teensy 3.2:  1, 5, 8, 10, 31
byte drawingMemory[num_of_leds * 3];       //  3 bytes per LED
DMAMEM byte displayMemory[num_of_leds * 12]; // 12 bytes per LED
WS2812Serial leds(num_of_leds, displayMemory, drawingMemory, pin, WS2812_GRB);

//1.0 is VERY bright if you're powering it off of 5V
// this needs to be declared and set to something >0 for the LEDs to work
float max_brightness = 0.2;
//led biz end

float hue_bank[64];
int enable_bank[64];
int cursor_blink;
unsigned long current_time;
unsigned long prev[8]; //array of 8 variables named "prev"
int shift;
float set_hue, hue_pot; //both of the se vatrible will be floats
int xy_sel;
int xy_count;
int x_pot;
int y_pot;

int draw_button_pin = 0;
int erase_button_pin = 1;

int draw_button, erase_button, prev_erase_button, prev_draw_button, enable_state, erase_enable;
unsigned long erase_time, erase_count;

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  pinMode(draw_button_pin, INPUT_PULLUP); //must be donw when you're digitalReading a button
  pinMode(erase_button_pin, INPUT_PULLUP);

  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some
}


void loop() {
  current_time = millis();

  prev_draw_button = draw_button;
  draw_button = digitalRead(draw_button_pin);

  if (prev_draw_button == 1 && draw_button == 0) {
    enable_state = !enable_state;
  }

  prev_erase_button = erase_button;
  erase_button = digitalRead(erase_button_pin);

  if (prev_erase_button == 1 && erase_button == 0) {
    erase_time = current_time;
    //erase_count = 0;
  }
  if (erase_button == 0) {
    if (current_time - erase_time > 2000) {
      erase_enable = 1;
    }
  }

  if (current_time - prev[0] > 30) {
    prev[0] = current_time;
    Serial.println(erase_count);

    //its better to not put analogRead in the "bottom" of the loop
    // reading it more slowly will give less noise and we only need
    // to update it when we'd see the change it causes anyway

    hue_pot = (analogRead(A2) / 4095.0); //0.0 to 1

    x_pot = (analogRead(A0) / 4095.0) * 7;
    y_pot = (analogRead(A1) / 4095.0) * 7;
    xy_sel = x_pot + (y_pot * 8);
    //Serial.println(xy_sel);


    if (enable_state == 1) { //0 is being press, 1 is not
      hue_bank[xy_sel] = hue_pot;
      enable_bank[xy_sel] = 1;
    }

    if (erase_enable == 1) {
      for ( int j = 0; j < num_of_leds; j++) {
        enable_bank[j] = 0;
      }
      erase_enable = 0;
    }

    //x_count goes from 0-7 and so does y_count but since we have it arranged
    // with one for loop after another we get x_count=0 for y_count from 0-7,
    // then x_count=1 for y_count from 0-7 and so on
    // this way we can more easily deal with the two dimensional LED array

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-64
        set_pixel_HSV(xy_count, hue_bank[xy_count], 1, enable_bank[xy_count]); // turn everything off. otherwise the last "frame" swill still show

        if (xy_count == xy_sel) {
          set_hue = hue_pot;
          if (cursor_blink == 0) {
            set_pixel_HSV(xy_sel, set_hue , 1, .2);
          }
          if (cursor_blink == 1) {
            set_pixel_HSV(xy_sel, set_hue , enable_state, 1);
          }
        }
      }
    }

    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function

  } //timing "if" is over

  if (current_time - prev[1] > 250) { //oscillate a varible so we can blink the cursor
    prev[1] = current_time;
    cursor_blink = !cursor_blink; //! is not, or opposite of. Only works on 0 and 1
  }

}// loop is over



//This function is a little different than you might see in other libraries but it works pretty similar
// instead of 0-255 you see in other libraries this is all 0-1.0
// you can copy this to the bottom of any code as long as the declarations at the top in "led biz" are done

//set_pixel_HSV(led to change, hue,saturation,value aka brightness)
// led to change is 0-63
// all other are 0.0 to 1.0
// hue - 0 is red, then through the ROYGBIV to 1.0 as red again
// saturation - 0 is fully white, 1 is fully colored.
// value - 0 is off, 1 is the value set by max_brightness
// (it's not called brightness since, unlike in photoshop, we're going from black to fully lit up

//based on https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

void set_pixel_HSV(int pixel, float fh, float fs, float fv) {
  byte RedLight;
  byte GreenLight;
  byte BlueLight;

  byte h = fh * 255;
  byte s = fs * 255;
  byte v = fv * max_brightness * 255;

  h = (h * 192) / 256;  // 0..191
  unsigned int i = h / 32;   // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;   // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;  // 0 -> 0xff, 0xff -> 0
  unsigned int fInv = 255 - f;  // 0 -> 0xff, 0xff -> 0
  byte pv = v * sInv / 256;  // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      RedLight = v;
      GreenLight = tv;
      BlueLight = pv;
      break;
    case 1:
      RedLight = qv;
      GreenLight = v;
      BlueLight = pv;
      break;
    case 2:
      RedLight = pv;
      GreenLight = v;
      BlueLight = tv;
      break;
    case 3:
      RedLight = pv;
      GreenLight = qv;
      BlueLight = v;
      break;
    case 4:
      RedLight = tv;
      GreenLight = pv;
      BlueLight = v;
      break;
    case 5:
      RedLight = v;
      GreenLight = pv;
      BlueLight = qv;
      break;
  }
  leds.setPixelColor(pixel, RedLight, GreenLight, BlueLight);
}
