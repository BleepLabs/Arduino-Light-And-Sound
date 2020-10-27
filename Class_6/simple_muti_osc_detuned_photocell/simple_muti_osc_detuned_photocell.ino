#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform4;      //xy=113.00568008422852,368.00564527511597
AudioSynthWaveform       waveform1;      //xy=115.00569152832031,181.00567150115967
AudioSynthWaveform       waveform3;      //xy=115.00568389892578,296.00567626953125
AudioSynthWaveform       waveform2;      //xy=117.00568389892578,237.00567626953125
AudioMixer4              mixer1;         //xy=388.0056381225586,223.00566482543945
AudioFilterStateVariable filter1;        //xy=553.0056190490723,222.00562477111816
AudioOutputAnalog        dac1;           //xy=715.0056838989258,309.0056686401367
AudioConnection          patchCord1(waveform4, 0, mixer1, 3);
AudioConnection          patchCord2(waveform1, 0, mixer1, 0);
AudioConnection          patchCord3(waveform3, 0, mixer1, 2);
AudioConnection          patchCord4(waveform2, 0, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, filter1, 0);
AudioConnection          patchCord6(filter1, 0, dac1, 0);
// GUItool: end automatically generated code

float pcell;
unsigned long current_time;
unsigned long prev_time[8];

void setup() {

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

  filter1.frequency(15000);
  filter1.resonance(.7);

  mixer1.gain(0, .2); // (channel 0-3, gain 0 is off, .5 full, 1 no change, 2 double and so on
  mixer1.gain(1, .2);
  mixer1.gain(2, .2);
  mixer1.gain(3, .2);
}

void loop() {
  current_time = millis();

  if (current_time - prev_time[0] > 5) {
    prev_time[0] = current_time;
    pcell = analogRead(A6) / 10.0;
    float freq1 = pcell * .5;
    float freq2 = pcell * 2;
    float freq3 = pcell * .999;
    waveform1.frequency(pcell);
    waveform2.frequency(freq1);
    waveform3.frequency(freq2);
    waveform4.frequency(freq3);

  }
  if (current_time - prev_time[1] > 50) {
    prev_time[1] = current_time;
    Serial.println(pcell);

  }
}
