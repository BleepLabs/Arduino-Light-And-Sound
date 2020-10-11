int raw_reading;
int interpolated_output;

unsigned long current_time;
unsigned long prev_time[8];


void setup() {
  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(32);  //smooth the readings some
}

void loop() {
  current_time = micros(); //read microseconds intead of millseconds

  //read and print at one speed
  if (current_time - prev_time[0] > 20 * 1000) { //20 milliseconds*1000 to make it micros
    prev_time[0] = current_time;

    raw_reading = analogRead(A0);

    Serial.print(4095); //print this upper value so the auto ranging serial plotter is more clear
    Serial.print(" "); // print a sapce between values so ther are difernt lines in the plotter
    Serial.print(raw_reading);
    Serial.print(" ");
    Serial.println(interpolated_output); //println means new line

  }

  //do the interpolation much more quickly
  if (current_time - prev_time[1] > 250) {
    prev_time[1] = current_time;

    if (raw_reading < interpolated_output) {
      interpolated_output--;
    }

    if (raw_reading > interpolated_output) {
      interpolated_output++;
    }
  }
}
