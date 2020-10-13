/*  roaming chaos influencer on autopilot
 *  praise eris or something
 *  creates a random set of pixels around a guide pixel, fading them towards guide pixel color
 *  all colors naturally fade, as all things do
 *  A3 pot controls decay rate
 *  A2 pot controls speed
 *  A1 pot controls saturation of guide pixel
 *  A0 pot controls hue of guide pixel
*/
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
float hue_val = 0.0;
float sat_val = 0.0;
float hue_rate;
float sat_rate;
int hue_dir = 1;
int sat_dir = 1;
int x_pot = 3;
int y_pot = 3;
int rate1 = 60;
int rate2;
int x_range = 2;
int y_range = 2;
int rand_x;
int rand_y;
int x_vel = 1;
int y_vel = 2;
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

    rate2 = analogRead(bottom_left_pot_pin)/8;

    decay_rate = analogRead(bottom_right_pot_pin)/4096.0;
    button_state = digitalRead(left_button_pin);

    if(current_time - prev_time[1] > rate2) {
      prev_time[1] = current_time;
      x_pot += x_vel;
      y_pot += y_vel;
  
      if(x_pot > 6) {
        x_pot = 6;
        x_vel = -random(1,2);
      }
      if(y_pot > 6) {
        y_pot = 6;
        y_vel = -random(1,2);
      }
      if(x_pot < 1) {
        x_pot = 1;
        x_vel = random(1,2);
      }
      if(y_pot < 1) {
        y_pot = 1;
        y_vel = random(1,2);
      }
    }
    
    hue_rate = analogRead(top_left_pot_pin)/40950.0;
    hue_val = hue_val + hue_rate * hue_dir;
    if(hue_val > 1) {
      hue_val = 1;
      hue_dir *= -1;
    }
    if(hue_val < 0) {
      hue_val = 0;
      hue_dir *= -1;
    }

    sat_rate = analogRead(top_right_pot_pin)/40950.0;
    sat_val = sat_val + sat_rate * sat_dir;
    if(sat_val > 1) {
      sat_val = 1;
      sat_dir *= -1;
    }
    if(sat_val < 0) {
      sat_val = 0;
      sat_dir *= -1;
    }

    Serial.print("Hue Value: ");
    Serial.println(hue_val);
    Serial.print("Sat Value: ");
    Serial.println(sat_val);

    // fade all existing lights
    for(int i=0; i<64; i++) {
        smap[i] = max(0,smap[i]-decay_rate);
    }

    for(int i=0; i<25; i++) {
      rand_x = min(max(0,x_pot + random(-1,2)*kindaRandom()),7);
      rand_y = min(max(0,y_pot + random(-1,2)*kindaRandom()),7);
      
      
      xy_sel = rand_x + 8*rand_y;
      pot_sel = x_pot + 8*y_pot;
      
      hmap[xy_sel] = (hmap[xy_sel]+hmap[pot_sel])/2;
      vmap[xy_sel] = (vmap[xy_sel]+vmap[pot_sel])/2;
      smap[xy_sel] = (smap[xy_sel]+smap[pot_sel])/2;
      
      hmap[pot_sel] = hue_val;
      vmap[pot_sel] = sat_val;
      smap[pot_sel] = 0.8;
    }
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

// semi random number, 70% chance of 1, 20% chance of 2, 10% chance of 3
int kindaRandom() {
  int seed = random(1, 11);
  if(seed <=8) {
    return 1;
  }
  else {
    return 2;
  }
}

// positive or negative randomizer.. probably a better way to do this?
int randomSign() {
  int seed = random(1, 3);
  if(seed == 1) {
    return 1;
  }
  if(seed == 2) {
    return -1;
  }
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
