//Using an array to store colors

//led biz begin. don't worry about anything in this section besides max_brightness
#include <WS2812Serial.h>
//we'll be using the Teensy audio library and it doesn't play nicely with neopixels.h or fastled
// so Paul of PJRC made this much more efficient version
#define num_of_leds 64
#define led_data_pin 5 // only these pins can be used on the Teensy 3.2:  1, 5, 8, 10, 31
byte drawingMemory[num_of_leds * 3];       //  3 bytes per LED
DMAMEM byte displayMemory[num_of_leds * 12]; // 12 bytes per LED
WS2812Serial leds(num_of_leds, displayMemory, drawingMemory, led_data_pin, WS2812_GRB);

//1.0 is VERY bright if you're powering it off of 5V
// this needs to be declared and set to something >0 for the LEDs to work
float max_brightness = 0.1;
//led biz end

//defines are not variables and are best use for things like like pin numbers for now
#define left_button_pin 0
#define right_button_pin 1
#define top_left_pot_pin A0
#define top_right_pot_pin A1
#define bottom_left_pot_pin A2
#define bottom_right_pot_pin A3

unsigned long current_time;
unsigned long prev_time[8]; //array of 8 variables named "prev"

float set_hue;
int xy_sel;
int xy_count;
int x_pot;
int y_pot;
int rate1 = 30;
int rate2 = 250;
int rate3 = 500;

//I used a formula to make a pallet that was a kind of rotating triad
//
float hues[8] = {};
float sats[8] = {1, 1, .8, 1, 1, .8, 1, .8};
float brights[8] = {.5, 1, 1, 1, 1, 1, 1, .5};

int x_sel;
int y_sel;
int hue_move;
int array_index[4];
int lfo[4];
int lfo_latch[4];

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  // a little algorhythm to make a new palet
  // try cahnging these three numbers
  hues[0] = .3; //starting color
  float hue_increment = .2;

  for (int i = 1; i < 8; i ++) { //start at 1
    hues[i] = hues[i - 1] + hue_increment;

    //while is similar to if but it keeps doing it as long as the contionin is true
    // we don't want numbers to go over 1.0 and % won't work for floats
    // so we jsut keep subrating 1 if we're over 1
    // if we came in with 1.1 or 2.1 we'd exit with 0.1;
    while (hues[i] > 1.0) {
      hues[i] -= 1.0; //wrap it back around
    }
  }

  Serial.println("hue array:");
  for (int i = 0; i < 7; i ++) {
    Serial.print(hues[i], 4); //tells it to always print 4 decimal palces https://www.arduino.cc/reference/en/language/functions/communication/serial/print/
    Serial.print(" "); //print a space
  }
  Serial.println(); // make a new line

  pinMode(left_button_pin, INPUT_PULLUP); //must be done when reading buttons
  pinMode(right_button_pin, INPUT_PULLUP);
  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some
}


void loop() {
  current_time = millis();

  if (current_time - prev_time[2] > rate2) {
    prev_time[2] = current_time;

    lfo[1]++;
    if (lfo[1] > 7) {
      lfo[1] = 0;
    }

  }

  if (current_time - prev_time[0] > rate1) {
    prev_time[0] = current_time;

    //x_count goes from 0-7 and so does y_count but since we have it arranged
    // with one for loop after another we get x_count=0 for y_count from 0-7,
    // then x_count=1 for y_count from 0-7 and so on
    // this way we can more easily deal with the two dimensional LED array
    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-64
        set_pixel_HSV(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show

        if (y_count == 2) { //&& means and. A single & is a whole other thing
          //set_pixel_HSV(led to change, hue,saturation,value aka brightness)
          hue_move = lfo[1] + x_count;
          //we can't have hue go over 8 but we also don't want to jsut snap it back to 0 every time
          //% is remainder aka modulo. remainder = dividend % divisor. https://www.arduino.cc/reference/en/language/structure/arithmetic-operators/remainder/
          // hue_move will never equal or go over 8. It'll jsut keep wrapping around
          // 7%8= 7, 8%8=0, 9%8=1, 17%8=1.
          // you can use this calculator to try it out //https://www.wolframalpha.com/input/?i=9%257%3D
          hue_move %= 8;

          //set_pixel_HSV(xy_count, hues[hue_move], sats[x_count], brights[x_count]);
          set_pixel_HSV(xy_count, hues[hue_move], 1, 1);
        }
      }
    }
    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
    //timing "if" over
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
