/* -------------------------------------------
Use pots in A2 and A3 to control a pixel.
Try to match the other one three times.
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
int x_pos;
int y_pos;
int score = 0;
// initial target position
int target_pos = random(63);

// boilerplate setup
void setup() {
  leds.begin();

  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some
}

void loop() {
  current_time = millis();
    if (current_time - prev[0] > 60 ) {
    prev[0] = current_time;
   

    // get the player dot from pots
    x_pos = analogRead(A2)/512;
    y_pos = analogRead(A3)/512;
    
    // translate our x/y into absolute LED position
    xy_sel = x_pos + (y_pos * 8);
        
    for (int i = 0; i < 64; i++) {    
        set_pixel_HSV(i, 0, 0, 0);

        if (i == xy_sel) {
          set_pixel_HSV(i, 0.34 , 1, 1);
        }
        if (xy_sel == target_pos) {
          score++;
          target_pos = random(63);
        }
        if (i == target_pos) {
          set_hue = 1 - (random(10) / 1000.0);
          set_pixel_HSV(i, set_hue , 1, 1);
        }
        if (score >= 3) {
          wow(10);
          score = 0;
        }

      }
    leds.show();
    }
  }

void wow(int count) {
  int wow1[64] = {1, 0, 1, 0, 1, 0, 1, 0, 
                  1, 0, 1, 0, 1, 0, 1, 0, 
                  1, 0, 1, 0, 1, 0, 1, 0, 
                  1, 0, 1, 0, 1, 0, 1, 0, 
                  1, 0, 1, 0, 1, 0, 1, 0, 
                  0, 0, 0, 0, 0, 0, 0, 0, 
                  1, 0, 1, 0, 1, 0, 1, 0, 
                  0, 0, 0, 0, 0, 0, 0, 0};

  int wow2[64] = {0, 1, 0, 1, 0, 1, 0, 1, 
                  0, 1, 0, 1, 0, 1, 0, 1, 
                  0, 1, 0, 1, 0, 1, 0, 1, 
                  0, 1, 0, 1, 0, 1, 0, 1, 
                  0, 1, 0, 1, 0, 1, 0, 1, 
                  0, 0, 0, 0, 0, 0, 0, 0, 
                  0, 1, 0, 1, 0, 1, 0, 1, 
                  0, 0, 0, 0, 0, 0, 0, 0};
                  
  for(int repeat = 0; repeat < count; repeat++) {
    for(int i = 0; i < 64; i++) {
      set_pixel_HSV(i, 1 , 1, 1*wow1[i]);   
    }
    leds.show();
    delay(200);
    for(int i = 0; i < 64; i++) {
      set_pixel_HSV(i, 1 , 1, 1*wow2[i]);   
    }
    leds.show();
    delay(200); 
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
