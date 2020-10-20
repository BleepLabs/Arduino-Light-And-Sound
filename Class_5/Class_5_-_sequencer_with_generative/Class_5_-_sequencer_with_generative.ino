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
AudioSynthWaveform       waveform1;      //xy=167,575
AudioEffectEnvelope      envelope1;      //xy=339,579
AudioSynthSimpleDrum     drum1;          //xy=345,516
AudioMixer4              mixer1;         //xy=513,589
AudioOutputAnalog        dac1;           //xy=675,584
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(envelope1, 0, mixer1, 1);
AudioConnection          patchCord3(drum1, 0, mixer1, 0);
AudioConnection          patchCord4(mixer1, dac1);
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
int repetes;
//"const" means to store it in program memory, not RAM
const float chromatic[88] = {55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192};
int minor[7] = {0, 2, 3, 5, 7, 9, 10};

#define seq_length 16

byte sequencer_array[2][seq_length] =
{
  {
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
  },
  {
    12, 12, 12, 12, 12, 0, 15, 0, 0, 0, 15, 0, 17, 19, 0, 0,
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

  middle_button.attach(middle_button_pin); //what pin will it read
  middle_button.interval(10); //how many milliseconds of debounce time

  right_button.attach(right_button_pin); //what pin will it read
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
  waveform1.begin(WAVEFORM_TRIANGLE); //begin(waveshape) list of shapes here https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform1.amplitude(1); //amplitude from 0.0-1.0

  mixer1.gain(0, .5); // (channel 0-3, gain 0 is off, .5 full, 1 no change, 2 double and so on
  mixer1.gain(1, .5);
  drum1.length(120); //Length of sound in milliseconds
  drum1.frequency(300); //pitch in Hz

  envelope1.release(250); //how long will the note fade out in millis after button is released


}



void loop() {
  current_time = millis();

  left_button.update(); //check the debouncers
  middle_button.update();
  right_button.update();

  if (left_button.fell()) { //much simpler way of saying  prev_button == 1 && button == 0
    drum1.noteOn(); //only happens once and does not need a corresponding note off
  }

  if (middle_button.fell()) {
    envelope1.noteOn(); //only happens once
  }
  if (middle_button.rose()) {
    envelope1.noteOff(); //only happens once. If it don't happen the note will just stay on
  }



  if (current_time - prev_time[3] > seq_rate) {
    prev_time[3] = current_time;

    seq_position++; //the current position we're looking at in the sequencer_array
    if (seq_position > seq_length - 1) {
      seq_position = 0;
      repetes++;
      if (repetes > 2) {
        repetes = 0;
        for (int new_notes = 0; new_notes < seq_length; new_notes++) {
          int r1 = random(1, 11);
          if (r1 > 6) {
            sequencer_array[1][new_notes] = random(12, 30);
            Serial.print(sequencer_array[1][new_notes]);
            Serial.print(" ");
          }
          else {
            sequencer_array[1][new_notes] = 0;

          }
        }
        Serial.println(" ");
      }
    }
    if (sequencer_array[0][seq_position] > 0) {
      drum1.noteOn();
    }

    if (sequencer_array[1][seq_position] > 0) {
      float note = chromatic[sequencer_array[1][seq_position]];
      waveform1.frequency(note);

      if (prev_seq1 == 0) {
        envelope1.noteOn(); //only make a new note on if the previous step in the array was 0
      }
    }

    if (sequencer_array[1][seq_position] == 0) {
      if (prev_seq1 >= 0) {
        envelope1.noteOff(); //only tun a note off if the current step is 0 and the last one was greater than 0
      }
    }

    prev_seq1 = sequencer_array[1][seq_position];  //remember the current value for next time. We could also just look back in the array but this is easy

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

    drum1.frequency(freq[0]);

  }

  if (current_time - prev_time[1] > rate1) {
    prev_time[1] = current_time;

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-64
        set_pixel_HSV(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show
      }
    }

    for ( int seq_step = 0; seq_step < seq_length; seq_step++) { //just go through the sequencer
      array_state = sequencer_array[0][seq_step];

      if (array_state > 0) {
        //make a colorful dot if there's something in the array
        // move the pixel we're lighting up by 8
        set_pixel_HSV(seq_step + 8, .5 , 1 , 1);
      }
      if (array_state == 0) {
        set_pixel_HSV(seq_step + 8, 0 , 0 , .1); //..or make it dim white if it's empty
      }

      array_state = sequencer_array[1][seq_step];
      if (array_state > 0) {
        //set the color based on the note
        //there are 88 steps in the chromatic array but you really can't see a different in the values we're using so I just used a smaller value
        float color = array_state / 88.0;
        // move the pixel we're lighting up by 32
        set_pixel_HSV(seq_step + 32, color , 1 , 1);
      }
      if (array_state == 0) {
        set_pixel_HSV(seq_step + 32, 0 , 0 , .1);
      }
    }

    //light up the current positions medium white
    set_pixel_HSV(seq_position + 8, 0 , 0 , .5);
    set_pixel_HSV(seq_position + 32, 0 , 0 , .5);

    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
  } //timing "if" over


  if (current_time - prev_time[2] > 100 && 1 == 0) {
    prev_time[2] = current_time;
    Serial.print("seq_rate ");
    Serial.println(seq_rate);
    //Here we print out the usage of the audio library
    // If we go over 90% processor usage or get near the value of memory blocks we set aside in the setup we'll have issues or crash.
    // This pretty much all a copy paste
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
