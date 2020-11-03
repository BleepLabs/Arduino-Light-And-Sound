// Making two separate sounds and controlling them with buttons
// LED screen lights up when the buttons are used

// install this library by searching for "bounce2 " in  sketch>include library>manage libraries...
#include <Bounce2.h>
//info on what bounce does here https://github.com/thomasfredericks/Bounce2#alternate-debounce-algorithms-for-advanced-users-and-specific-cases
//functions https://github.com/thomasfredericks/Bounce2/wiki#methods
#define BOUNCE_LOCK_OUT //this tells it what mode to be in. I think it's the better one for music

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform3;      //xy=109.00568389892578,463.99999237060547
AudioSynthWaveform       waveform1;      //xy=118.00566482543945,218.00000762939453
AudioSynthWaveform       waveform2;      //xy=133.00568389892578,395.99999237060547
AudioSynthNoiseWhite     noise1;         //xy=151.00568389892578,333.99999237060547
AudioMixer4              mixer2;         //xy=210.00568771362305,524.9999628067017
AudioSynthSimpleDrum     drum2;          //xy=238.00568389892578,701.9999923706055
AudioEffectEnvelope      envelope1;      //xy=254.00565338134766,221.00000381469727
AudioSynthNoiseWhite     noise2;         //xy=261.0056571960449,627.0000019073486
AudioEffectEnvelope      envelope2;      //xy=353.0056610107422,445.99999237060547
AudioFilterStateVariable filter1;        //xy=370.0056610107422,310.9999828338623
AudioMixer4              mixer3;         //xy=410.0056838989258,660.9999923706055
AudioSynthSimpleDrum     drum1;          //xy=427.0056838989258,179.99999237060547
AudioEffectEnvelope      envelope3;      //xy=448.00565338134766,592.0000009536743
AudioMixer4              mixer1;         //xy=569.0056838989258,288.99998664855957
AudioOutputAnalog        dac1;           //xy=759.005615234375,303.9999755397439
AudioAnalyzeFFT256       fft256_1;       //xy=800.0055809020996,356.91475105285645
AudioAnalyzeRMS          rms1;           //xy=806.0056915283203,224.9147720336914
AudioConnection          patchCord1(waveform3, 0, mixer2, 2);
AudioConnection          patchCord2(waveform1, envelope1);
AudioConnection          patchCord3(waveform2, 0, mixer2, 0);
AudioConnection          patchCord4(noise1, 0, mixer2, 1);
AudioConnection          patchCord5(mixer2, envelope2);
AudioConnection          patchCord6(drum2, 0, mixer3, 1);
AudioConnection          patchCord7(envelope1, 0, filter1, 0);
AudioConnection          patchCord8(noise2, 0, mixer3, 0);
AudioConnection          patchCord9(envelope2, 0, mixer1, 2);
AudioConnection          patchCord10(filter1, 0, mixer1, 1);
AudioConnection          patchCord11(mixer3, envelope3);
AudioConnection          patchCord12(drum1, 0, mixer1, 0);
AudioConnection          patchCord13(envelope3, 0, mixer1, 3);
AudioConnection          patchCord14(mixer1, dac1);
AudioConnection          patchCord15(mixer1, rms1);
AudioConnection          patchCord16(mixer1, fft256_1);
AudioControlSGTL5000     sgtl5000_1;     //xy=72.00568389892578,73.99999237060547
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
#define BOUNCE_LOCK_OUT //this tells it what mode to be in. I think it's the better one for music

//Make and name bounce objects for each button we want to use
Bounce left_button = Bounce(); // to add more buttons just copy this and change "left_button" to somethign else
Bounce middle_button = Bounce();
Bounce right_button = Bounce();


unsigned long current_time;
unsigned long prev_time[8];

float set_hue;
int xy_sel;
int xy_count;
int rate1 = 30;
float freq[4];
int top_left_pot, top_right_pot, bottom_left_pot, bottom_right_pot;
int left_b, right_b, middle_b, prev_left_b, prev_right_b, prev_middle_b;
float fft_bank[8];


#define bitmap_width  8
#define bitmap_height 8

//arranging the array like this has makes no difference to the Teensy but makes it easy for us to make designs
// A byte is used to conserve RAM
// Math like this can only be done if the value is a #define. it won't work with variables
byte bitmap[bitmap_width * bitmap_height] =
{
  0, 0, 0, 1, 1, 0, 0, 0,
  0, 0, 1, 2, 2, 1, 0, 0,
  0, 1, 2, 3, 3, 2, 1, 0,
  1, 2, 3, 4, 4, 3, 2, 1,
  1, 2, 3, 4, 4, 3, 2, 1,
  0, 1, 2, 3, 3, 2, 1, 0,
  0, 0, 1, 2, 2, 1, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0,
};
float peak1_reading;
void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  pinMode(left_button_pin, INPUT_PULLUP); //must be done when reading buttons
  pinMode(right_button_pin, INPUT_PULLUP);
  pinMode(middle_button_pin, INPUT_PULLUP);

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
  AudioMemory(20);

  //we define all of these in the block of code we copied fro the tool
  //now we have to tell them what to do
  waveform1.begin(WAVEFORM_SINE); //begin(waveshape) //https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform1.amplitude(1); //amplitude from 0.0-1.0
  waveform2.begin(WAVEFORM_SAWTOOTH_REVERSE);
  waveform3.begin(WAVEFORM_SAWTOOTH);

  mixer1.gain(0, 5); // (channel 0-3, gain 0 is off, .5 full, 1 no change, 2 double and so on
  mixer1.gain(1, 5);
  mixer1.gain(2, 1);

  mixer2.gain(0, 3);
  mixer2.gain(1, 5);
  mixer2.gain(2, 5);
  mixer2.gain(3, 5);

  mixer3.gain(0, 5);
  mixer3.gain(1, 5);

  drum1.length(120); //Length of sound in milliseconds
  drum1.frequency(300); //pitch in Hz
  drum2.frequency(160);
  drum2.length(120);
  envelope1.release(250); //how long will the note fade out in millis after
  filter1.frequency(500);
  filter1.resonance(.7);
  noise1.amplitude(.2);
  noise2.amplitude(.5);
  waveform2.amplitude(1);
  waveform2.frequency(11500);
  waveform3.amplitude(1);
  waveform3.frequency(13500);

  //envelope2.release(1);
  envelope2.attack(5);
  envelope2.decay(10);
  envelope2.sustain(.5);
  envelope2.release(1200);
}



void loop() {
  current_time = millis();
  left_button.update(); //check the debouncers
  middle_button.update();
  right_button.update();


  if (left_button.fell()) { //much simpler way of saying  prev_button == 1 && button == 0
    drum1.noteOn(); //only happens once and does not need a corresponding note off
  }

  if (freq[1] >= 500) {
    envelope2.release (5);
  }
  if (freq[1] <= 500) {
    envelope2.release (1200);
  }

  if (middle_button.fell()) {
    envelope3.noteOn(); //only happens once


  }
  if (middle_button.rose()) {
    envelope3.noteOff(); //only happens once. If it don't happen the note will just stay on
  }
  if (right_button.fell()) {
    envelope2.noteOn(); //only happens once
  }
  if (right_button.rose()) {
    envelope2.noteOff(); //only happens once. If it don't happen the note will just stay on
  }


  if (current_time - prev_time[0] > 5) { // it's better to read a little more slowly the much faster bottom of the loop    prev_time[0] = current_time;
    prev_time[0] = current_time;
    top_left_pot = analogRead(top_left_pot_pin);
    top_right_pot = analogRead(top_right_pot_pin);
    bottom_left_pot = analogRead(bottom_left_pot_pin);
    bottom_right_pot = analogRead(bottom_right_pot_pin);

    freq[0] = bottom_left_pot / 6.0;
    freq[1] = bottom_right_pot / 6.0;
    freq[2] = (top_right_pot / 4095.0) * 1500.0;
    freq[4] = ((top_left_pot / 4095.0) * 5.0) + 0.7;

    drum1.frequency(freq[0]);
    waveform1.frequency(freq[1]);

    filter1.frequency(freq[2]);
    filter1.resonance(freq[4]);

  }

  if (current_time - prev_time[1] > rate1) {
    prev_time[1] = current_time;

    if (rms1.available()) {
      peak1_reading = rms1.read();
    }
    int fft_sel = 0;
    if (fft256_1.available()) {
      for ( int ff = 0; ff < 127; ff += 128 / 8) {
        fft_bank[fft_sel] = fft256_1.read(ff) * 10.0;
        if (fft_bank[fft_sel] > 1.0) {
          fft_bank[fft_sel] = 1.0;
        }
        fft_sel++;
      }
    }

    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-64
        set_pixel_HSV(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show

        //light up the corners "||" means or
        if (xy_count == 0 || xy_count == 7 || xy_count == 56 || xy_count == 63) {
          float r1 = random(100) / 500.0; //make a new floating point variable that will be filled with a random number between 0-.25
          set_pixel_HSV(xy_count, 0 , 4 , r1 + .1);

        }
        set_pixel_HSV(xy_count, .6 , 1 , fft_bank[y_count]);
      }
    }
    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function
  } //timing "if" over


  if (current_time - prev_time[3] > 30) {
    prev_time[3] = current_time;
    for ( int ff = 0; ff < 8; ff ++) {
      Serial.print(fft_bank[ff]);
      Serial.print(" ");
    }
    Serial.println();
  }


  if (current_time - prev_time[2] > 500 && 1 == 0) {
    prev_time[2] = current_time;

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
