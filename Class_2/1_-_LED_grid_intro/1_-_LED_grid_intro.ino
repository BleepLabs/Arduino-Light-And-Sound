/*
Starting out with addressable LEDs

You'll need to install this library https://github.com/PaulStoffregen/WS2812Serial
 Click the green code button and download the ZIP
 In Arduino click sketch>include library>add zip library... ad give it the one you downloaded 


The LED screen has three things that need to be connected using your long jumper wires
 Din connects to pin 5 of your Teensy
 +5V can connect to the 5V pin on the top left corer of the Teensy 
  but 3.3V, anywhere on the red breadboard line, works too and is less bright. 
 GND connects to ground, anywhere on the blue breadboard line

The other three pins on the screen can be used to chain more LED device 

*/

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
float max_brightness = 0.1;
//led biz end


unsigned long current_time;
unsigned long prev[8]; //array of 8 variables named "prev"
int shift;
float set_hue;

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some
}


void loop() {
  current_time = millis();

  if (current_time - prev[0] > 60) {
    prev[0] = current_time;

    shift++;
    if (shift > num_of_leds) { //goes up to 64 then down to 0;
      shift = 0;
    }

    for ( int i = 0; i < num_of_leds; i++) { // this block of code will repeat 64 times with i going from 0-63

      int led_sel = shift + i;
      if (led_sel > num_of_leds - 1) {
        //combine i and shift so the rainbow moves
        // if it goes over the number of LEDs we subtract 64 rather than setting it to 0
        // as it might be over 64 and then we'd skip some values
        // for example if it ends up being 70 we want it to go back to 6, not 0
        led_sel -= num_of_leds;
      }

      //set_pixel_HSV(led to change, hue 0.0-1.0, saturation 0.0-1.0, value aka brightness 0.0-1.0)
      // more info below
      // for hue we cant just say num_of_leds as it's not a float and won't divide properly. You could also do float(num_of_leds)
      set_hue = i / 64.0;
      set_pixel_HSV(led_sel, set_hue, 1.0, 1.0);
    }

    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function

  }
} // loop is over




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
