// Sequencer for two sounds and LED display

// install this library by searching for "bounce2 " in  sketch>include library>manage libraries...
#include <Bounce2.h>
//info on what bounce does here https://github.com/thomasfredericks/Bounce2#alternate-debounce-algorithms-for-advanced-users-and-specific-cases
//functions https://github.com/thomasfredericks/Bounce2/wiki#methods
#define BOUNCE_LOCK_OUT //this tells it what mode to be in. I think it's the better one for music

//copied from the audio tool https://www.pjrc.com/teensy/gui/
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum2;          //xy=150.00569534301758,226.0056667327881
AudioSynthNoiseWhite     noise1;         //xy=152.0056915283203,295.0056586265564
AudioSynthSimpleDrum     drum1;          //xy=241.00564575195312,171.99995708465576
AudioFilterStateVariable filter1;        //xy=289.00561904907227,327.00556993484497
AudioMixer4              mixer1;         //xy=454.00562286376953,237.99996757507324
AudioOutputAnalog        dac1;           //xy=620.0056838989258,244.99999237060547
AudioConnection          patchCord1(drum2, 0, mixer1, 1);
AudioConnection          patchCord2(noise1, 0, filter1, 0);
AudioConnection          patchCord3(drum1, 0, mixer1, 0);
AudioConnection          patchCord4(filter1, 0, mixer1, 2);
AudioConnection          patchCord5(mixer1, dac1);
// GUItool: end automatically generated code


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
#define middle_button_pin 1
#define right_button_pin 2
#define top_left_pot_pin A0
#define top_right_pot_pin A1
#define bottom_left_pot_pin A2
#define bottom_right_pot_pin A3

//Make and name bounce objects for each button we want to use
Bounce left_button = Bounce(); // to add more buttons just copy this and change "left_button" to somethign else
Bounce middle_button = Bounce();
Bounce right_button = Bounce();

unsigned long current_time;
unsigned long prev_time[8];

int xy_sel;
int xy_count;
int rate1 = 30;
float freq[4];
int top_left_pot, top_right_pot, bottom_left_pot, bottom_right_pot; //all of these are separate ints
int left_b, right_b, middle_b, prev_left_b, prev_right_b, prev_middle_b;
int seq_rate;
int seq_position;
int prev_seq1;
int array_state;

//"const" means to store it in program memory, not RAM
const float chromatic[88] = {55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192};

#define seq_length 16

byte sequencer_array[2][seq_length] =
{
  {
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
  },
  {
    0, 0, 0, 0, 12, 12, 12, 15, 0, 0, 15, 0, 17, 19, 0, 0,
  },
};

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  pinMode(left_button_pin, INPUT_PULLUP); //must be done when reading buttons
  pinMode(right_button_pin, INPUT_PULLUP);
  pinMode(middle_button_pin, INPUT_PULLUP);

  //setting up debouncers
  left_button.attach(left_button_pin); //what pin will it read
  left_button.interval(10); //how many milliseconds of debounce time

  middle_button.attach(right_button_pin); //what pin will it read
  middle_button.interval(10); //how many milliseconds of debounce time

  right_button.attach(middle_button_pin); //what pin will it read
  right_button.interval(10); //how many milliseconds of debounce time


  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some

  //This must be done to start the audio library running
  //We must set aside a certain amount of memory for the audio code to use
  //The easies way to do this is start with 10 and then if the print out of memory usage
  //goes above that, simply increase this value until you're a over the vale its reporting
  AudioMemory(10);

  //we define all of these in the block of code we copied fro the tool
  //now we have to tell them what to do

  mixer1.gain(0, .3); // (channel 0-3, gain 0 is off, .5 full, 1 no change, 2 double and so on
  mixer1.gain(1, .3);
  mixer1.gain(2, .4);

  drum1.length(50); //Length of sound in milliseconds
  drum1.frequency(100); //pitch in Hz
  drum2.length(50); //Length of sound in milliseconds
  drum2.frequency(175); //pitch in Hz
  filter1.frequency(0);
  filter1.resonance(.5);
  noise1.amplitude(1);

}

float filter_env;
int filter_latch;
unsigned long filter_timer;

void loop() {
  current_time = millis();

  left_button.update(); //check the debouncers
  middle_button.update();
  right_button.update();
  float r1=random(100,400);
  drum1.frequency(r1); //pitch in Hz
  
  if (left_button.fell()) { //much simpler way of saying  prev_button == 1 && button == 0
    drum1.noteOn(); //only happens once and does not need a corresponding note off
    drum2.noteOn();
    filter_latch = 1;
    filter_timer = current_time;
    filter_env = 15000;
    filter1.frequency(filter_env);
  }
  if (filter_latch == 1) {
    filter_env *= .995;
    if (filter_env < 100) {
      filter_env = 0;
      filter_latch = 0;
    }
    filter1.frequency(filter_env);

  }


  if (middle_button.fell()) {
  }
  if (middle_button.rose()) {
  }



  if (current_time - prev_time[3] > seq_rate) {
    prev_time[3] = current_time;

  }

  if (current_time - prev_time[0] > 5) { // it's better to read a little more slowly the much faster bottom of the loop
    prev_time[0] = current_time;

    top_left_pot = analogRead(top_left_pot_pin);
    top_right_pot = analogRead(top_right_pot_pin);
    bottom_left_pot = analogRead(bottom_left_pot_pin);
    bottom_right_pot = analogRead(bottom_right_pot_pin);

    freq[0] = bottom_left_pot / 6.0;
    freq[1] = bottom_right_pot / 6.0;
    seq_rate = top_left_pot / 16.0;

    // drum1.frequency(freq[0]);

  }



  if (current_time - prev_time[2] > 100) {
    prev_time[2] = current_time;

    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();

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
