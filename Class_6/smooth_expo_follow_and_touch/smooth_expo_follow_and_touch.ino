/*
  In this sketch several examples are shown:
  Smoothing analog readings
  Maping values into expo or log curves
  Reading touch sensors
  Using envelope followers

*/

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
  //smooth needs to happed fast so read and smooth in the bottom of the loop, not in a timing "if"
  raw_reading[0] = analogRead(A0);

  //smooth(select, num of readings, input);
  // select needs to be diffect for every differnt varible you want to smooth
  // number of readings can be anything from 7-99. The larger the number the more smooth but the less responsive
  smoothed_reading[0] = smooth(0, 25, raw_reading[0]);

  //fscale(input value,  input min, input max, ouput min, output max, curve) {
  // works like map but allows for float results and applying a log or expo curve
  // If curve is 0, it returns a linear scaling
  // less than 0 and it's logarithmic
  // greater than 0 and it's exponential

  // for the min and max in I printed out the smoothed readings and saw waht the actaul vales were
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
    //follower[1]=moothed_reading[1]; //or jump to it immediatley
  }

  if (follower[1] > smoothed_reading[1]) {
    follower[1] *= fall_rate; //slowly falles to the new reading after it has dropped
  }

  //------Printer
  //printing alwasy needs to be in a timing if. Don't go faster than 5 milliseconds
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
      //Serial.print(raw_reading[1]);
      //  Serial.print(" ");
      // Serial.print(smoothed_reading[1]);
      //  Serial.print(" ");
      Serial.print(follower[1]);
      Serial.println();
    }
  }

} //end of loop


//~~~~ Functions bellow ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//based on https://playground.arduino.cc/Main/DigitalSmooth/
// This function continuously samples an input and puts it in an array that is "samples" in length.
// This array has a new "raw_in" value added to it each time "smooth" is called and an old value is removed
// It throws out the top and bottom 15% of readings and averages the rest

#define maxarrays 8 //max number of different variables to smooth
#define maxsamples 99 //max number of points to sample and 
//reduce these numbers to save RAM

unsigned int smoothArray[maxarrays][maxsamples];

// sel should be a unique number for each occurrence
// samples should be an odd number greater that 7. It's the length of the array. The larger the more smooth but less responsive
// raw_in is the input. positive numbers in and out only.

unsigned int smooth(byte sel, unsigned int samples, unsigned int raw_in) {
  int j, k, temp, top, bottom;
  long total;
  static int i[maxarrays];
  static int sorted[maxarrays][maxsamples];
  boolean done;

  i[sel] = (i[sel] + 1) % samples;    // increment counter and roll over if necessary. -  % (modulo operator) rolls over variable
  smoothArray[sel][i[sel]] = raw_in;                 // input new data into the oldest slot

  for (j = 0; j < samples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[sel][j] = smoothArray[sel][j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (samples - 1); j++) {
      if (sorted[sel][j] > sorted[sel][j + 1]) {    // numbers are out of order - swap
        temp = sorted[sel][j + 1];
        sorted[sel] [j + 1] =  sorted[sel][j] ;
        sorted[sel] [j] = temp;
        done = 0;
      }
    }
  }

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((samples * 15)  / 100), 1);
  top = min((((samples * 85) / 100) + 1  ), (samples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j < top; j++) {
    total += sorted[sel][j];  // total remaining indices
    k++;
  }
  return total / k;    // divide by number of samples
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//floating point scaling
// works like map but allows for float results and applying a log or expo curve
// If curve is 0, it returns a linear scaling
// less than 0 and it's logarithmic
// greater than 0 and it's exponential

// based on
///https://playground.arduino.cc/Main/Fscale/
// there are more efficient ways of doing this but it works just fine for getting pots to other ranges.

float fscale(float inputValue,  float originalMin, float originalMax, float newBegin, float
             newEnd, float curve) {

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  if (curve > 1) curve = 1;
  if (curve < -1) curve = -1;

  float curve_amount = 1.0; // increase this number to get steeper curves
  curve = (curve * curve_amount * -1.0) ; // - invert and scale - this seems more intuitive - positive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into logarithmic exponent for other pow function


  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero reference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin) {
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0) {
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
