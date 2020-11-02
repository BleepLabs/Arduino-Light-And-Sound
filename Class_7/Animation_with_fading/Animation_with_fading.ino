//Using arrays as bitmaps
// a bitmap is just that, an uncompressed map of bits
// a bmp file used 3 bytes per pixel to represent the colors
// here we're using a single value per "color" and then deciding how to display each one


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
float max_brightness = 0.2;
//led biz end

//defines are not variables
// but unlike varibles they can be used in the declaration section and don't take up any memory
#define left_button_pin 0
#define right_button_pin 1
#define top_left_pot_pin A0
#define top_right_pot_pin A1
#define bottom_left_pot_pin A2
#define bottom_right_pot_pin A3

unsigned long current_time;
unsigned long prev_time[8];
int glyph_location;
float set_hue;
int xy_sel;
int xy_count;
int x_pot;
int y_pot;
int rate1 = 30;
int g_sel;
int pot[4];
int animation_rate;
int animation_frame, next_frame;
float fade, inv_fade;

#define bitmap_width  8
#define bitmap_height 8
#define total_frames 3

//aranging the array like this has makes no difference to the teensy but makes it easy for us to make designs
// A byte is used to conserve RAM
// Math like this can only be done if the value is a #define. it won't work with varables

byte bitmap[total_frames][bitmap_width * bitmap_height] =
{
  {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 0, 0, 0, 0, 3, 0,
    0, 0, 0, 2, 2, 0, 0, 0,
    0, 0, 2, 0, 0, 3, 0, 0,
    0, 0, 2, 0, 0, 3, 0, 0,
    0, 0, 0, 2, 2, 0, 0, 0,
    0, 3, 0, 0, 0, 0, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  },
  {
    3, 0, 0, 0, 0, 0, 0, 3,
    0, 0, 4, 5, 5, 4, 0, 0,
    0, 4, 0, 0, 0, 0, 4, 0,
    8, 5, 0, 0, 0, 0, 5, 8,
    8, 5, 0, 0, 0, 0, 5, 8,
    0, 4, 0, 0, 0, 0, 4, 0,
    0, 0, 4, 5, 5, 4, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 3,
  }
};

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  pinMode(left_button_pin, INPUT_PULLUP); //must be done when reading buttons
  pinMode(right_button_pin, INPUT_PULLUP);
  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some
}


void loop() {
  current_time = millis();

  if (current_time - prev_time[2] > 5) { //fast timer to read controls
    prev_time[2] = current_time;
    pot[1] = (analogRead(top_left_pot_pin) / 4095.0) * 2000;
    animation_rate = pot[1];
    fade *= .98;
    if (fade < .1) {
      fade = 0;
    }
    inv_fade = 1.0 - fade;
  }

  if (current_time - prev_time[1] > animation_rate) { //fast timer to read controls
    prev_time[1] = current_time;
    animation_frame++;
    if (animation_frame > total_frames - 1) {
      animation_frame = 0;
    }
    next_frame = animation_frame + 1;
    if (next_frame > total_frames - 1) {
      next_frame = 0;
    }
    fade = 1;
  }

  if (current_time - prev_time[0] > rate1) {
    prev_time[0] = current_time;

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-64
        set_pixel_HSV(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show

        int current_pixel = bitmap[animation_frame][xy_count];
        int next_pixel = bitmap[next_frame][xy_count];

        if (current_pixel > 0 ) {
          float color1 = current_pixel / 8.0;
          set_pixel_HSV(xy_count, color1 , 1, fade);
        }
        if (next_pixel > 0 ) {
          float color1 = next_pixel / 8.0;
          set_pixel_HSV(xy_count, color1 , 1, inv_fade);
        }
      }
    }
    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
  } //timing "if" over

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
