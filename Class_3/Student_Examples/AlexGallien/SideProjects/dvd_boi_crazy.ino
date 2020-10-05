/* -------------------------------------------
Bouncy square like the classic DVD screensaver
with a slightly more random pattern. Speed
controlled by pot in A3 by default.
-------------------------------------------- */

// boilerplate LED init
#include <WS2812Serial.h> 
const int num_of_leds = 64;
const int pin = 5; // only these pins can be used on the Teensy 3.2:  1, 5, 8, 10, 31
byte drawingMemory[num_of_leds * 3];       //  3 bytes per LED
DMAMEM byte displayMemory[num_of_leds * 12]; // 12 bytes per LED
WS2812Serial leds(num_of_leds, displayMemory, drawingMemory, pin, WS2812_GRB);
float max_brightness = 0.1;

unsigned long current_time;
unsigned long interval_time;
unsigned long prev[8]; //array of 8 variables named "prev"
int shift;
float set_hue;
int xy_sel;
int xy_count;

// initial x/y position of top left pixel (0,6 for each)
int x_pos = 2;
int y_pos = 0;

// initial x/y velocity
int x_vel = 1;
int y_vel = 1;

// boilerplate setup
void setup() {
  leds.begin();

  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some
}

void loop() {
  current_time = millis();
  
  // get the time from a pot so you can control speed
  interval_time = analogRead(A3)/4;
  
  if (current_time - prev[0] > interval_time ) {
    prev[0] = current_time;

    // changing velocity at 'walls' for top left pixel
    // adds some randomness otherwise it's always a repeating pattern
    if (x_pos >= 6) {
      x_vel = -1 * random(2);
    }
    if (x_pos <= 0) {
      x_vel = 1 * random(2);
    }
    if (y_pos >= 6) {
      y_vel = -1 * random(2);
    }
    if (y_pos <= 0) {
      y_vel = 1 * random(2);
    }

    // translate our x/y into absolute LED position
    xy_sel = x_pos + (y_pos * 8);
        
    for (int i = 0; i < 64; i++) {    
        set_pixel_HSV(i, 0, 0, 0);

        // Create square relative to top left pixel
        if (i == xy_sel) {
          set_hue = .4 + (random(10) / 40.0);
          set_pixel_HSV(i, set_hue , .9, 1);
        }
        if (i == xy_sel+1) {
          set_hue = .4 + (random(10) / 40.0);
          set_pixel_HSV(i, set_hue , .9, 1);
        }
        if (i == xy_sel+8) {
          set_hue = .4 + (random(10) / 40.0);
          set_pixel_HSV(i, set_hue , .9, 1);
        }
        if (i == xy_sel+9) {
          set_hue = .4 + (random(10) / 40.0);
          set_pixel_HSV(i, set_hue , .9, 1);
        }
        

    }
  leds.show();

  // adjust position of guide pixel by velocity
  x_pos+=x_vel;
  y_pos+=y_vel;
  }

}

// Magic below

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
