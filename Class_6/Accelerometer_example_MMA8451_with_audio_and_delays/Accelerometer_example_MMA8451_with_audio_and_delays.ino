/*
  Using the Bleep labs MMA8451 accelerometer
  the board we're using is similar to this https://www.adafruit.com/product/2019 but I made it a long time ago before it was available from adafruit but never put it in a product
*/
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum4;          //xy=157.6966667175293,322.31212520599365
AudioSynthSimpleDrum     drum3;          //xy=164.61977005004883,274.61971855163574
AudioSynthWaveform       waveform1;      //xy=173.85053253173828,376.7875680923462
AudioSynthSimpleDrum     drum2;          //xy=177.69669723510742,226.92746543884277
AudioSynthWaveform       waveform2;      //xy=183.0812759399414,431.40295600891113
AudioSynthSimpleDrum     drum1;          //xy=193.8505096435547,182.31204986572266
AudioSynthWaveform       waveform3;      //xy=203.85054397583008,486.7875509262085
AudioMixer4              mixer2;         //xy=372.3119888305664,356.01830101013184
AudioMixer4              mixer1;         //xy=394.3132438659668,253.92870330810547
AudioEffectDelay         delay1;         //xy=585.3889579772949,407.62668228149414
AudioMixer4              mixer3;         //xy=590.7734909057617,285.24913597106934
AudioOutputAnalog        dac1;           //xy=609.7747230529785,608.1594066619873
AudioConnection          patchCord1(drum4, 0, mixer1, 3);
AudioConnection          patchCord2(drum3, 0, mixer1, 2);
AudioConnection          patchCord3(waveform1, 0, mixer2, 0);
AudioConnection          patchCord4(drum2, 0, mixer1, 1);
AudioConnection          patchCord5(waveform2, 0, mixer2, 1);
AudioConnection          patchCord6(drum1, 0, mixer1, 0);
AudioConnection          patchCord7(waveform3, 0, mixer2, 2);
AudioConnection          patchCord8(mixer2, 0, mixer3, 1);
AudioConnection          patchCord9(mixer1, 0, mixer3, 0);
AudioConnection          patchCord10(delay1, 0, dac1, 0);
AudioConnection          patchCord11(delay1, 0, mixer3, 2);
AudioConnection          patchCord12(mixer3, delay1);
// GUItool: end automatically generated code



#include "smooth_n_scale.h" //instead of the functions being at the bottom of this code they are in a tab at the top. we jsut need to include it

//these must be included for the accelo to work.
// Find them in sketch>include library > manage libraries and search for "MMA8451" and "Adafruit unified Sensor" which will be at the bottom of the list
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

//wire aka i2c communication is already on your computer
#include <Wire.h>

//name the sensor and get the library working
Adafruit_MMA8451 accelo = Adafruit_MMA8451();

unsigned long current_time, prev[4];
int x_read, y_read, z_read;
int orientation, prev_orientation;
int  smoothed_x;
int  smoothed_y;
int  smoothed_z;
int freq_x;
int freq_y;
int freq_z;

void setup(void) {
  delay(100);

  while (!Serial) { //while no serial communication...
    //...don't do anything until you open the serial monitor
  }

  Serial.println("~Adafruit library MMA8451 test~");

  //if the accelo doesn't begin...
  if (!accelo.begin(0x1C)) { //necessary for this board. adafruit board has different address
    Serial.println("Couldnt start. Check connections");
    delay(500);
    while (1); //..dont continue with code if it can't find it.
  }
  Serial.println("MMA8451 found!");

  //can be "MMA8451_RANGE_2_G", "MMA8451_RANGE_4_G" or "MMA8451_RANGE_8_G"
  // the bigger the number,the less sensitive
  accelo.setRange(MMA8451_RANGE_2_G);

  AudioMemory(100);

  mixer1.gain(0, .25);
  mixer1.gain(1, .25);
  mixer1.gain(2, .25);
  mixer1.gain(3, .25);

  mixer2.gain(0, 1);
  mixer2.gain(1, 0);
  mixer2.gain(2, 0);
  mixer2.gain(3, 0);

  mixer3.gain(0, .5);
  mixer3.gain(1, .5);
  mixer3.gain(2, 0);
  mixer3.gain(3, 0);

  drum1.length(120); //Length of sound in milliseconds
  drum1.frequency(327); //pitch in Hz

  drum2.length(100); //Length of sound in milliseconds
  drum2.frequency(641); //pitch in Hz

  drum3.length(200); //Length of sound in milliseconds
  drum3.frequency(220); //pitch in Hz

  drum4.length(200); //Length of sound in milliseconds
  drum4.frequency(440); //pitch in Hz

  waveform1.begin(WAVEFORM_TRIANGLE); //begin(waveshape) //https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform1.amplitude(1); //amplitude from 0.0-1.0
  waveform2.begin(WAVEFORM_TRIANGLE); //begin(waveshape) //https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform2.amplitude(1); //amplitude from 0.0-1.0
  waveform3.begin(WAVEFORM_TRIANGLE); //begin(waveshape) //https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform3.amplitude(1); //amplitude from 0.0-1.0

  delay1.delay(0, 150);
  delay1.delay(1, 100);

  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some


}

void loop() {
  current_time = millis();


  if (current_time - prev[2] > 500) {
    prev[2] = current_time;

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

  //read and smooth quickly

  // Read the raw data. fills the x y z with bipolar 14 bit values, -8191 to 8191

  accelo.read();

  //we don't need all that resolution so lets divide it down to -1000 to 1000 to make it easier to understand
  // since 8191 is our biggest number just move the decimal in that and divide by it
  x_read = accelo.x / 8.191; //these can only be done after accelo.read()
  y_read = accelo.y / 8.191;
  z_read = accelo.z / 8.191;

  float fb = analogRead(A0) / 4095.0;
  mixer3.gain(2, fb);
  mixer3.gain(3, fb);

  //smooth(select, number of readings, input);
  // select needs to be different for each variable you want to smooth
  // number of readings can be anything from 7-99 and should be odd for best results.
  // The larger the number the more smooth but the less responsive the result will be


  smoothed_x = smooth(0, 51, x_read);
  smoothed_y = smooth(1, 51, y_read);
  smoothed_z = smooth(2, 51, z_read);
  freq_x = map(smoothed_x, -1000, 1000, 100, 1000);
  waveform1.frequency(freq_x);
  freq_y = map(smoothed_y, -1000, 1000, 0, 1000);
  waveform2.frequency(freq_y);
  freq_z = map(smoothed_y, -1000, 1000, 0, 1000);
  waveform3.frequency(freq_z);

  if (current_time - prev[0] > 40) {
    prev[0] = current_time;

    Serial.println(freq_x);


    int print_smooth = 2;

    if (print_smooth == 0) {
      Serial.print(x_read);
      Serial.print(" ");
      Serial.print(y_read);
      Serial.print(" ");
      Serial.print(z_read);
      Serial.println();
    }

    if (print_smooth == 1) {
      Serial.print(smoothed_x);
      Serial.print(" ");
      Serial.print(smoothed_y);
      Serial.print(" ");
      Serial.print(smoothed_z);
      Serial.println();
    }


    //this device works just like the accelo in your phone that rotates the screen.
    // this function gets a value from 0-7 which then we print as a phone based orientation.
    prev_orientation = orientation;
    orientation = accelo.getOrientation();

    //"switch case" is similar to "if" but you give it a pile of possible outcomes and it picks just one
    // then it "breaks", leaving the switch and going on to the next thing
    // https://www.arduino.cc/reference/en/language/structure/control-structure/switchcase/





    switch (orientation) {
      //these are defines in the adafruit library. "#define MMA8451_PL_PUF 0" https://github.com/adafruit/Adafruit_MMA8451_Library/blob/c7f64f04f00a16b6c786677db4fc75eec65fabdd/Adafruit_MMA8451.h#L45
      // so this is just the same as saying:
      // case 0:
      case MMA8451_PL_PUF:
        if (prev_orientation != orientation) {
          drum1.noteOn();
        }
        Serial.println("Portrait Up Front");
        //you could put whatever you want here. maybe it could trigger different sounds
        break;
      case MMA8451_PL_PUB:
        if (prev_orientation != orientation) {
          drum2.noteOn();
        }
        Serial.println("Portrait Up Back");
        break;
      case MMA8451_PL_PDF:
        if (prev_orientation != orientation) {
          drum3.noteOn();
        }
        Serial.println("Portrait Down Front");
        break;
      case MMA8451_PL_PDB:
        if (prev_orientation != orientation) {
          drum4.noteOn();
        }
        Serial.println("Portrait Down Back");
        break;
      case MMA8451_PL_LRF:
        Serial.println("Landscape Right Front");
        break;
      case MMA8451_PL_LRB:
        Serial.println("Landscape Right Back");
        break;
      case MMA8451_PL_LLF:
        Serial.println("Landscape Left Front");
        break;
      case MMA8451_PL_LLB:
        Serial.println("Landscape Left Back");
        break;
    }
    Serial.println(); // print a return to space it out

  }



} // end of loop
