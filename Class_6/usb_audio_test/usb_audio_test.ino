//USB audio input to Teensy analog audio out

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=280.0056915283203,289.0056838989258
AudioMixer4              mixer1;         //xy=471.0056915283203,302.0056838989258
AudioAnalyzePeak         peak1;          //xy=607.0056915283203,210.00568389892578
AudioOutputAnalog        dac1;           //xy=715.0056838989258,309.0056686401367
AudioConnection          patchCord1(usb1, 0, mixer1, 0);
AudioConnection          patchCord2(usb1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, dac1);
AudioConnection          patchCord4(mixer1, peak1);
// GUItool: end automatically generated code

unsigned long current_time;
unsigned long prev_time[8];

void setup() {
  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some

  AudioMemory(10);
  
  //only pass the left channel along to the teensy audio output
  // you can combine both but it might sound odd and phased out
  mixer1.gain(0, 1); 
  mixer1.gain(1, 0);
}

void loop() {
  current_time = millis();

  if (current_time - prev_time[2] > 100) {
    prev_time[2] = current_time;
    //peak returns the max audio level sicne it was last checked 
    // but needs to be asked if it's avaiable first
    if (peak1.available()) {
      Serial.print("input level: ");
      Serial.println(peak1.read()); 
    }
    else {
      // if it's not avaiale then most probablt the computer is not connected to the 
      // Teensys USB input
      Serial.println("no USB audio connection ");
    }
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();

  }


}
