/*
  Using the Bleep labs MMA8451 accelerometer
  the board we're using is similar to this https://www.adafruit.com/product/2019 but I made it a long time ago before it was available from adafruit but never put it in a product
*/


//usb midi ref https://www.pjrc.com/teensy/td_midi.html

#include "smooth_n_scale.h" //instead of the functions being at the bottom of this code they are in a tab at the top. we jsut need to include it

//these must be included for the accelo to work.
// Find them in sketch>include library > manage libraries and search for "MMA8451" and "Adafruit unified Sensor" which will be at the bottom of the list
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

//wire aka i2c communication is already on your computer
#include <Wire.h>

//name the snesor and get the library working
Adafruit_MMA8451 accelo = Adafruit_MMA8451();

unsigned long current_time, prev[4];
int x_read, y_read, z_read;
int orientation, prev_orientation;
int  smoothed_x;
int  smoothed_y;
int  smoothed_z;
int z_out, prev_z_out;

int note_latch[4];
unsigned long note_latch_time[4];

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

  smoothed_x = smooth(0, 41, x_read);
  smoothed_y = smooth(1, 41, y_read);
  smoothed_z = smooth(2, 41, z_read);

  prev_z_out = z_out;
  z_out = map(smoothed_z, -1000, 1000, 0, 127); //midi is 0-127

  if (prev_z_out != z_out) {
    usbMIDI.sendControlChange(20, z_out, 1);
  }





  if (current_time - prev[0] > 40) {
    prev[0] = current_time;

    int print_smooth = 1;

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
    // this function gets a value from 0-8 which then we print as a phone based orientation.
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
        Serial.println("Portrait Up Front");
        if (prev_orientation != orientation) {
          usbMIDI.sendNoteOn(40, 127, 1);
          note_latch[0] = 1;
          note_latch_time[0] = current_time;
        }
        //you could put whatever you want here. maybe it could trigger different sounds
        break;
      case MMA8451_PL_PUB:
        Serial.println("Portrait Up Back");
        break;
      case MMA8451_PL_PDF:
        Serial.println("Portrait Down Front");
        break;
      case MMA8451_PL_PDB:
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

  if (note_latch[0] == 1) {
    if (current_time - note_latch_time[0] > 1000) {
      usbMIDI.sendNoteOff(40, 0, 1);
      note_latch[0] = 0;
    }
  }

} // end of loop
