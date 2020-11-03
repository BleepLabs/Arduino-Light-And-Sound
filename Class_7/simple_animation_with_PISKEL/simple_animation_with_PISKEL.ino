// Using piskel to make animations www.piskelapp.com
// start by selecting your cavans size using the resize button on the right
// 8x8 is the easisest but you can use any size. Look at "Class_4_-_bitmap_3_large_text" for info on scrolling it

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
float max_brightness = .1;
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
int animation_step;


//in piskel go to export on the right side menu
// select other and "donwload C file"
// in that file copy the array here and change the values between the "[]" as shown
// then jsut cahnge your frame rate asn size to match

#define bitmap_width 8
#define bitmap_height 8
#define total_frames 7

static const uint32_t piskel_output[total_frames][bitmap_width * bitmap_height] = { //uint32_t is the same as unsigned long
  {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff00ff53, 0xff00ff53, 0x00000000, 0x00000000, 0x00000000
  },
  {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffff8300, 0xffff8300, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff00ff53, 0xff00ff53, 0x00000000, 0x00000000, 0x00000000
  },
  {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffdd00, 0x00000000, 0x00000000, 0xffffdd00, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xffff8300, 0xffff8300, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff00ff53, 0xff00ff53, 0x00000000, 0x00000000, 0x00000000
  },
  {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffff00b8, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff00b8, 0x00000000,
    0x00000000, 0x00000000, 0xffffdd00, 0x00000000, 0x00000000, 0xffffdd00, 0x00000000, 0x00000000,
    0x00000000, 0xffff00b8, 0x00000000, 0xffff8300, 0xffff8300, 0x00000000, 0xffff00b8, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xff00ff53, 0xff00ff53, 0x00000000, 0x00000000, 0x00000000
  },
  {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff7c00ff, 0x00000000, 0xff7c00ff, 0x00000000, 0x00000000, 0xff7c00ff, 0x00000000, 0xff7c00ff,
    0x00000000, 0xffff00b8, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff00b8, 0x00000000,
    0x00000000, 0x00000000, 0xffffdd00, 0x00000000, 0x00000000, 0xffffdd00, 0x00000000, 0x00000000,
    0x00000000, 0xffff00b8, 0x00000000, 0xffff8300, 0xffff8300, 0x00000000, 0xffff00b8, 0x00000000,
    0xff7c00ff, 0x00000000, 0x00000000, 0xff00ff53, 0xff00ff53, 0x00000000, 0x00000000, 0xff7c00ff
  },
  {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xff009dff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff009dff,
    0x00000000, 0xff009dff, 0x00000000, 0xff009dff, 0xff009dff, 0x00000000, 0xff009dff, 0x00000000,
    0xff7c00ff, 0x00000000, 0xff7c00ff, 0x00000000, 0x00000000, 0xff7c00ff, 0x00000000, 0xff7c00ff,
    0x00000000, 0xffff00b8, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff00b8, 0x00000000,
    0xff009dff, 0x00000000, 0xffffdd00, 0x00000000, 0x00000000, 0xffffdd00, 0x00000000, 0xff009dff,
    0x00000000, 0xffff00b8, 0x00000000, 0xffff8300, 0xffff8300, 0x00000000, 0xffff00b8, 0x00000000,
    0xff7c00ff, 0x00000000, 0x00000000, 0xff00ff53, 0xff00ff53, 0x00000000, 0x00000000, 0xff7c00ff
  },
  {
    0x00000000, 0xff66fdf4, 0x00000000, 0xff66fdf4, 0xff66fdf4, 0x00000000, 0xff66fdf4, 0x00000000,
    0xff009dff, 0x00000000, 0xff66fdf4, 0x00000000, 0x00000000, 0xff66fdf4, 0x00000000, 0xff009dff,
    0x00000000, 0xff009dff, 0x00000000, 0xff009dff, 0xff009dff, 0x00000000, 0xff009dff, 0x00000000,
    0xff7c00ff, 0x00000000, 0xff7c00ff, 0x00000000, 0x00000000, 0xff7c00ff, 0x00000000, 0xff7c00ff,
    0x00000000, 0xffff00b8, 0x00000000, 0xff66fdf4, 0xff66fdf4, 0x00000000, 0xffff00b8, 0x00000000,
    0xff009dff, 0x00000000, 0xffffdd00, 0x00000000, 0x00000000, 0xffffdd00, 0x00000000, 0xff009dff,
    0x00000000, 0xffff00b8, 0x00000000, 0xffff8300, 0xffff8300, 0x00000000, 0xffff00b8, 0x00000000,
    0xff7c00ff, 0x00000000, 0x00000000, 0xff00ff53, 0xff00ff53, 0x00000000, 0x00000000, 0xff7c00ff
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

  if (current_time - prev_time[1] > 500) {
    prev_time[1] = current_time;
    animation_step = animation_step + 1;
    if (animation_step > total_frames - 1) {
      animation_step = 0;
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

        ////needs to be uint32_t aka unsigned long as thats what piskel exports
        uint32_t current_bitmap_pixel = piskel_output[animation_step][xy_count];

        //set_PISKEL(led number ,piskel color in uint32_t)
        //use this isntead of set_pixel_HSV
        set_PISKEL(xy_count, current_bitmap_pixel);

        //you can still to set_pixel_HSV in thes same code though
        if (y_count == 7) {
          if (x_count > 2 && x_count < 5 ) {
            float c1 = random(100) / 100.0;
            set_pixel_HSV(xy_count, c1, 1, .33);
          }
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

void set_PISKEL(int pixel, uint32_t c32) {
  //data is stored as 8bits of Alpha (transparency which is ignored) then 8bits blue, 8bits green, 8bits red
  byte RedLight = (c32); //jsut the bottom 8 bits the rest gets chopped of fsince its a byte
  byte GreenLight = (c32 >> 8); //bit shift by 8 so now only the green is in the 8 bits of the byte
  byte BlueLight = (c32 >> 16);
  leds.setPixelColor(pixel, RedLight * max_brightness, GreenLight * max_brightness, BlueLight * max_brightness);
}

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
