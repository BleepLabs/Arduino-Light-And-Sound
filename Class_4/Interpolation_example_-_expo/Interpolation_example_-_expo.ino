// Interpolation info https://github.com/BleepLabs/Arduino-Light-And-Sound/wiki/Interpolation

float raw_reading = 1;
float interpolated_output = 1;

unsigned long current_time;
unsigned long prev_time[8];
float increase_jump_size = 1.001;
float decrease_jump_size = .995; //it will decrease faster

void setup() {
  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(32);  //smooth the readings some
  raw_reading = analogRead(A0) + 1; //it cant be 0, otherwise we won't be able to multiply multiply 
  interpolated_output = raw_reading; //start by jumping right to the pot's position

}

void loop() {
  current_time = micros(); //read microseconds intead of millseconds

  //read and print at one speed
  if (current_time - prev_time[0] > 20* 1000) { //20 milliseconds*1000 to make it micros
    prev_time[0] = current_time;

    raw_reading = analogRead(A0) + 1;

    Serial.print(4095); //print this upper value so the auto ranging serial plotter is more clear
    Serial.print(" "); // print a sapce between values so ther are difernt lines in the plotter
    Serial.print(raw_reading);
    Serial.print(" ");
    Serial.println(interpolated_output); //println means new line

  }

  //do the interpolation much more quickly
  if (current_time - prev_time[1] > 500) { //you can make it go much faster, here I've slowed it down so you can see it more easily
    prev_time[1] = current_time;
    
    if (raw_reading < interpolated_output) {
      interpolated_output *= decrease_jump_size;
    }

    if (raw_reading > interpolated_output) {
      interpolated_output *= increase_jump_size;
    }
  }
}
