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
AudioSynthWaveform       waveform1;      //xy=520,542
AudioSynthWaveform       waveform3;      //xy=520,661
AudioSynthWaveform       waveform2;      //xy=522,602
AudioSynthWaveform       waveform4;      //xy=526,718
AudioFilterStateVariable filter2;        //xy=654,603
AudioFilterStateVariable filter3;        //xy=654,662
AudioFilterStateVariable filter4;        //xy=659,723
AudioFilterStateVariable filter1;        //xy=666,542
AudioEffectEnvelope      envelope4;      //xy=806,714
AudioEffectEnvelope      envelope3;      //xy=807,647
AudioEffectEnvelope      envelope2;      //xy=810,598
AudioEffectEnvelope      envelope1;      //xy=824,537
AudioMixer4              mixer1;         //xy=1032,596
AudioOutputAnalog        dac1;           //xy=1317,592
AudioConnection          patchCord1(waveform1, 0, filter1, 0);
AudioConnection          patchCord2(waveform3, 0, filter3, 0);
AudioConnection          patchCord3(waveform2, 0, filter2, 0);
AudioConnection          patchCord4(waveform4, 0, filter4, 0);
AudioConnection          patchCord5(filter2, 0, envelope2, 0);
AudioConnection          patchCord6(filter3, 0, envelope3, 0);
AudioConnection          patchCord7(filter4, 1, envelope4, 0);
AudioConnection          patchCord8(filter1, 0, envelope1, 0);
AudioConnection          patchCord9(envelope4, 0, mixer1, 3);
AudioConnection          patchCord10(envelope3, 0, mixer1, 2);
AudioConnection          patchCord11(envelope2, 0, mixer1, 1);
AudioConnection          patchCord12(envelope1, 0, mixer1, 0);
AudioConnection          patchCord13(mixer1, dac1);
// GUItool: end automatically generated code

#define left_button_pin 0
#define middle_button_pin 1
#define right_button_pin 2
#define top_left_pot_pin A0
#define top_right_pot_pin A1
#define bottom_left_pot_pin A2
#define bottom_right_pot_pin A3

Bounce left_button = Bounce();
Bounce middle_button = Bounce();
Bounce right_button = Bounce();

//Piano array
const float chromatic[88] = {55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192};
int scale[16] = {29, 29, 29, 29, 31, 32, 32, 32, 34, 36, 36, 37, 39, 39, 41, 41};
int rand_note;
int chords[4][3] = {
  {17, 21, 25}, // Dm6
  {15, 19, 24}, // Db6
  {12, 15, 22}, // Am7
  {12, 17, 21}, // Dm/A?
};
int chord_inc = 0;
float freq[3];
float res[3];
unsigned long current_time;
unsigned long prev_time[8];
int top_left_pot, top_right_pot, bottom_left_pot, bottom_right_pot;
int note_latch[4];
unsigned long note_latch_time[4];
void setup() {
  pinMode(left_button_pin, INPUT_PULLUP); //must be done when reading buttons
  pinMode(right_button_pin, INPUT_PULLUP);
  pinMode(middle_button_pin, INPUT_PULLUP);

  left_button.attach(left_button_pin);
  left_button.interval(10);
  middle_button.attach(middle_button_pin);
  middle_button.interval(10);
  right_button.attach(right_button_pin);
  right_button.interval(10);

  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);
  AudioMemory(10);

  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(1); //amplitude from 0.0-1.0
  waveform2.begin(WAVEFORM_SAWTOOTH);
  waveform2.amplitude(1);
  waveform3.begin(WAVEFORM_SAWTOOTH);
  waveform3.amplitude(1);
  waveform4.begin(WAVEFORM_SAWTOOTH);
  waveform4.amplitude(1);

  mixer1.gain(0, .2); // (channel 0-3, gain 0 is off, .5 full, 1 no change, 2 double and so on
  mixer1.gain(1, .2);
  mixer1.gain(2, .2);
  mixer1.gain(3, .2);

}

void loop() {
  current_time = millis();

  left_button.update(); //check the debouncers
  middle_button.update();
  right_button.update();

  if (current_time - prev_time[0] > 2400) {
    prev_time[0] = current_time;

    waveform1.frequency(chromatic[chords[chord_inc % 4][0]]); // modulo is just for lazy incrementing
    waveform2.frequency(chromatic[chords[chord_inc % 4][1]]);
    waveform3.frequency(chromatic[chords[chord_inc % 4][2]]);
    envelope1.noteOn();
    envelope2.noteOn();
    envelope3.noteOn();
    note_latch[0] = 1;
    note_latch_time[0] = current_time;
    chord_inc += 1;
  }

  if (note_latch[0] == 1) {
    if (current_time - note_latch_time[0] > 1000) {
      envelope1.noteOff();
      envelope2.noteOff();
      envelope3.noteOff();
      note_latch[0] = 0;
    }
  }




  if (current_time - prev_time[1] > 600) { // quarter note

    prev_time[1] = current_time;
    rand_note = random(0, 16);
    Serial.println(rand_note);
    waveform4.frequency(chromatic[scale[rand_note]]);
    envelope4.noteOn();
    note_latch[1] = 1;
    note_latch_time[1] = current_time;
  }


  if (note_latch[1] == 1) {
    if (current_time - note_latch_time[1] > 100) {
      envelope4.noteOff();
      note_latch[1] = 0;
    }
  }


  if (current_time - prev_time[2] > 5) { // fast but not too fast
    prev_time[2] = current_time;

    top_left_pot = analogRead(top_left_pot_pin);
    top_right_pot = analogRead(top_right_pot_pin);
    bottom_left_pot = analogRead(bottom_left_pot_pin);
    bottom_right_pot = analogRead(bottom_right_pot_pin);

    freq[0] = bottom_left_pot / 6.0;
    freq[1] = chromatic[scale[rand_note]] + (top_left_pot / 3.0);
    res[0] = (bottom_right_pot / 4096.0) * 3.0;
    res[1] = (top_right_pot / 4096.0) * 3.0;
    filter1.frequency(freq[0]);
    filter2.frequency(freq[0]);
    filter3.frequency(freq[0]);
    filter4.frequency(freq[1]);

    filter1.resonance(res[0]);
    filter2.resonance(res[0]);
    filter3.resonance(res[0]);
    filter4.resonance(res[1]);
  }
}
