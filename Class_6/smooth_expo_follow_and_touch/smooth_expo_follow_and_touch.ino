/*
  In this sketch several examples are shown:
  Smoothing analog readings
  Mapping values into expo or log curves
  Reading touch sensors
  Using envelope followers
*/

#include "smooth_scale.h" //instead of the functions being at the bottom of this code they are in a tab at the top. we jsut need to include it

int  raw_reading[4];
int  smoothed_reading[4];
float follower[4] = {1, 1, 1, 1}; //can't be 0 since we can't multiply by it

float fall_rate = .995; //very small changes in these values make a big difference
float rise_rate = 1.01;

unsigned long current_time, prev[4];
float expo_out, log_out;

void setup() {
  analogReadResolution(12);
  analogReadAveraging(32); //this should still be used. it happed at the hardware level and is a different kind of averaging.
}

void loop() {
  current_time = millis();

  //------Pot scaling
  raw_reading[0] = analogRead(A0);

  //smooth(select, number of readings, input);
  // select needs to be different for each variable you want to smooth
  // number of readings can be anything from 7-99 and should be odd for best results.
  // The larger the number the more smooth but the less responsive the result will be

  //smooth needs to happen fast so read and smooth in the bottom of the loop, not in a timing "if"
  // if you put it in a timing if the output will be very slow to respond
  smoothed_reading[0] = smooth(0, 25, raw_reading[0]);

  //fscale(input value,  input min, input max, ouput min, output max, curve) {
  // works like map but allows for float results and applying a log or expo curve
  // If curve is 0, it returns a linear scaling
  // less than 0 and it's logarithmic
  // greater than 0 and it's exponential

  // for the min and max in I printed out the smoothed readings and saw what the actual vales were
  expo_out = fscale(smoothed_reading[0], 29, 4075, 0, 4095, 0.5);
  log_out =  fscale(smoothed_reading[0], 29, 4075, 0, 4095, -0.5);

  //------Touch reading
  raw_reading[1] = touchRead(0) / 4; //touch read can return some large numbers so it might best to divide it before smoothing
  // see the notes here on making touch sensors https://github.com/BleepLabs/Arduino-Light-And-Sound/wiki/Touch-sensors
  smoothed_reading[1] = smooth(1, 47, raw_reading[1]);

  //------follower
  raw_reading[2] = analogRead(A1);
  smoothed_reading[2] = smooth(2, 25, raw_reading[2]);
  float fall_rate_adj = fscale(smoothed_reading[2], 29, 4075, 0, .1, 0.5);
  //fall_rate=1.00-fall_rate_adj;

  if (follower[1] < smoothed_reading[1]) {
    follower[1] *= rise_rate; //the follow jumps up to meet a new reading at one rate
    //follower[1]=moothed_reading[1]; //or jump to it immediately
  }

  if (follower[1] > smoothed_reading[1]) {
    follower[1] *= fall_rate; //slowly falls to the new reading after it has dropped
  }

  //------Printer
  //printing always needs to be in a timing if. Don't go faster than 5 milliseconds
  if (current_time - prev[0] > 40) {
    prev[0] = current_time;

    byte print_sel = 1; //which info to print?

    if (print_sel == 0) { //pot and expo
      Serial.print(smoothed_reading[0]);
      Serial.print(" ");
      Serial.print(expo_out);
      Serial.print(" ");
      Serial.print(log_out);
      Serial.println();
    }

    if (print_sel == 1) { //touch and follower
      // Serial.print(raw_reading[1]);
      // Serial.print(" ");
      // Serial.print(smoothed_reading[1]);
      // Serial.print(" ");
      Serial.print(follower[1]);
      Serial.println();
    }
  }

} //end of loop
