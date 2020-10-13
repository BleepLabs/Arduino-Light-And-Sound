
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
int pot_sel;
int xy_count;
float h_pot;
float s_pot;
int x_pot = 3;
int y_pot = 3;
int rate1 = 60;
int x_range = 2;
int y_range = 2;
int rand_x;
int rand_y;
int button_state = 1;
float decay_rate = 0.01;

float hmap[64] = {0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0};

float smap[64] = {0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0};

float vmap[64] = {0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0};


void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  pinMode(left_button_pin, INPUT_PULLUP); //must be done when reading buttons
  pinMode(right_button_pin, INPUT_PULLUP);
  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some
}


void loop() {
  current_time = millis();

  if (current_time - prev_time[0] > rate1) {
    prev_time[0] = current_time;

    button_state = digitalRead(left_button_pin);
    
    x_pot = analogRead(bottom_left_pot_pin)/512;
    y_pot = analogRead(bottom_right_pot_pin)/512;
    
    h_pot = analogRead(top_left_pot_pin)/4095.0;
    s_pot = analogRead(top_right_pot_pin)/4095.0;
    
    // fade all existing lights
    for(int i=0; i<64; i++) {
        smap[i] = max(0,smap[i]-decay_rate);
    }
    
    rand_x = min(max(0,x_pot + random(-x_range,x_range+1)),7);
    rand_y = min(max(0,y_pot + random(-y_range,y_range+1)),7);
    Serial.println(rand_x);
    Serial.println(rand_y);
    
    xy_sel = rand_x + 8*rand_y;
    pot_sel = x_pot + 8*y_pot;
    
    hmap[xy_sel] = (hmap[xy_sel]+hmap[pot_sel])/2;
    vmap[xy_sel] = (vmap[xy_sel]+vmap[pot_sel])/2;
    smap[xy_sel] = (smap[xy_sel]+smap[pot_sel])/2;
    
    hmap[pot_sel] = h_pot;
    vmap[pot_sel] = s_pot;
    smap[pot_sel] = 0.8;

    //render screen using maps
    for(int i=0; i<64; i++) {
      set_pixel_HSV(i, hmap[i], vmap[i], smap[i]);
    }

    // clear screen when button is pushed
    if(button_state==0) {
      for(int i=0; i<64; i++) {
        hmap[i] = 0;
        vmap[i] = 0;
        smap[i] = 0;
      }
      button_state = 1;
    }
    
    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
  } //timing "if" over

}// loop is over


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
