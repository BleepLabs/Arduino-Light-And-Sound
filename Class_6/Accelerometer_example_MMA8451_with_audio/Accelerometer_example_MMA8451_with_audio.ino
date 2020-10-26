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
AudioSynthWaveform       waveform1;      //xy=203.85053194486173,438.3260462834284
AudioSynthWaveform       waveform2;      //xy=206.15822425255405,479.8645078218899
AudioSynthSimpleDrum     drum2;          //xy=231.5428352355957,262.31208515167236
AudioSynthSimpleDrum     drum1;          //xy=238.46591186523438,200.0043716430664
AudioSynthWaveform       waveform3;      //xy=244.6197627140925,521.4029693603514
AudioSynthSimpleDrum     drum3;          //xy=248.46591186523438,310.0043783187866
AudioSynthSimpleDrum     drum4;          //xy=268.46591656024634,353.0812953068659
AudioMixer4              mixer1;         //xy=442.7749137878418,265.46722412109375
AudioMixer4              mixer2;         //xy=461.5428047180176,434.4798831939697
AudioMixer4              mixer3;         //xy=610.7735824584961,319.0952606201172
AudioOutputAnalog        dac1;           //xy=666.6978912353516,424.31329345703125
AudioConnection          patchCord1(waveform1, 0, mixer2, 0);
AudioConnection          patchCord2(waveform2, 0, mixer2, 1);
AudioConnection          patchCord3(drum2, 0, mixer1, 1);
AudioConnection          patchCord4(drum1, 0, mixer1, 0);
AudioConnection          patchCord5(waveform3, 0, mixer2, 2);
AudioConnection          patchCord6(drum3, 0, mixer1, 2);
AudioConnection          patchCord7(drum4, 0, mixer1, 3);
AudioConnection          patchCord8(mixer1, 0, mixer3, 0);
AudioConnection          patchCord9(mixer2, 0, mixer3, 1);
AudioConnection          patchCord10(mixer3, dac1);
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

  AudioMemory(10);

  mixer1.gain(0, .25);
  mixer1.gain(1, .25);
  mixer1.gain(2, .25);
  mixer1.gain(3, .25);

  mixer2.gain(0, .33);
  mixer2.gain(1, .33);
  mixer2.gain(2, .33);
  mixer2.gain(3, 0);

  mixer3.gain(0, 0);
  mixer3.gain(1, 1);
  mixer3.gain(2, 0);
  mixer3.gain(3, 0);

  drum1.length(120); //Length of sound in milliseconds
  drum1.frequency(327); //pitch in Hz

  drum2.length(1000); //Length of sound in milliseconds
  drum2.frequency(641); //pitch in Hz

  drum3.length(500); //Length of sound in milliseconds
  drum3.frequency(220); //pitch in Hz

  drum4.length(2500); //Length of sound in milliseconds
  drum4.frequency(440); //pitch in Hz

  waveform1.begin(WAVEFORM_TRIANGLE); //begin(waveshape) //https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform1.amplitude(1); //amplitude from 0.0-1.0
  waveform2.begin(WAVEFORM_TRIANGLE); //begin(waveshape) //https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform2.amplitude(1); //amplitude from 0.0-1.0
  waveform3.begin(WAVEFORM_TRIANGLE); //begin(waveshape) //https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform3.amplitude(1); //amplitude from 0.0-1.0

}

void loop() {
  current_time = millis();

  //read and smooth quickly

  // Read the raw data. fills the x y z with bipolar 14 bit values, -8191 to 8191
  accelo.read();

  //we don't need all that resolution so lets divide it down to -1000 to 1000 to make it easier to understand
  // since 8191 is our biggest number just move the decimal in that and divide by it
  x_read = accelo.x / 8.191; //these can only be done after accelo.read()
  y_read = accelo.y / 8.191;
  z_read = accelo.z / 8.191;


  //smooth(select, number of readings, input);
  // select needs to be different for each variable you want to smooth
  // number of readings can be anything from 7-99 and should be odd for best results.
  // The larger the number the more smooth but the less responsive the result will be


  smoothed_x = smooth(0, 51, x_read);
  smoothed_y = smooth(1, 51, y_read);
  smoothed_z = smooth(2, 51, z_read);
  freq_x=map(smoothed_x,-1000,1000,100,1000);
  waveform1.frequency(freq_x);
  freq_y=map(smoothed_y,-1000,1000,0,1000);
  waveform2.frequency(freq_y);
  freq_z=map(smoothed_y,-1000,1000,0,1000);
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
