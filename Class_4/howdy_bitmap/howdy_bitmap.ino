/*
 Examples created for my fall 2020 intro to Arduino course
 
 todo: comments!

This one is of a simple bitmap that can scrolls and loops. 
Colors are scroll speed are controlled with the knobs. 
Rather than creating sound directly this one sends MIDI notes and CCs to VCV rack.
 
 */
 
#include <WS2812Serial.h>

const int numled = 64;
const int pin = 5;
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
byte drawingMemory[numled * 3];       //  3 bytes per LED
DMAMEM byte displayMemory[numled * 12]; // 12 bytes per LED

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB);

const float chromatic[88] = {55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192};
byte major[65] = {1, 3, 5, 6, 8, 10, 12, 13, 15, 17, 18, 20, 22, 24, 25, 27, 29, 30, 32, 34, 36, 37, 39, 41, 42, 44, 46, 48, 49, 51, 53, 54, 56, 58, 60, 61, 63, 65, 66, 68, 70, 72, 73, 75, 77, 78, 80, 82, 84, 85, 87, 89, 90, 92, 94, 96, 97, 99, 101, 102};

//Then we have our varible declarations like before
unsigned long cm;
unsigned long prev[8];
float flfo[8], freq[8], amp[8];
int lfo[8], prev_lfo[8], osc[8], out[8];
float test1;
unsigned long cu, du, looptime;
int jj;
int xy1, xy2;
byte trails[20];
byte tc;
float fade[8];
byte newnote;
#define delay_len 10000
short delaybank[delay_len];
byte rh[8] = {24, 24, 24, 24};
int dx;
byte seq_step[4];
int seq_step_lerp[4];
byte arp_dir = 1;
byte arp_top[4];
int pot[8];
float ff[8];
int seq_rate;
int scrollx, scrolly;
int latchx, latchy;
int scrollc;
byte text[1024] = {

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 1, 0, 0, 2, 0, 0, 3, 0, 3, 0, 3, 0, 4, 4, 0, 0, 5, 0, 5, 0, 0,
  0, 1, 0, 1, 0, 2, 0, 2, 0, 3, 0, 3, 0, 3, 0, 4, 0, 4, 0, 5, 0, 5, 0, 0,
  0, 1, 1, 1, 0, 2, 0, 2, 0, 3, 0, 3, 0, 3, 0, 4, 0, 4, 0, 0, 5, 0, 0, 0,
  0, 1, 0, 1, 0, 2, 0, 2, 0, 0, 3, 0, 3, 0, 0, 4, 0, 4, 0, 0, 5, 0, 0, 0,
  0, 1, 0, 1, 0, 0, 2, 0, 0, 0, 3, 0, 3, 0, 0, 4, 4, 0, 0, 0, 5, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

};

byte n1 = 1;
byte tick = 0;

void setup() {

  leds.begin();
  pinMode(0, INPUT_PULLUP);


  analogWriteResolution(12);
  analogReadResolution(12);
  analogReadAveraging(16);

  //env1.trig();
  //env2.trig();
  prev[2] = 50;
}

void loop() {
  cm = millis();

  if (cm - prev[2] > 33 * pot[0]) {
    prev[2] = cm;
    usbMIDI.sendNoteOff(40, 0, 1);
    usbMIDI.sendNoteOff(40, 0, 2);
    usbMIDI.sendNoteOff(40, 0, 3);

    tick++;
    if (tick > 3) {
      tick = 0;
      usbMIDI.sendNoteOn(40, random(100, 127), 3);

    }

    scrollc += 16;

    int r1 = random(100);
    if (r1 > 20) {
      n1 = 1;
    }
    if (r1 > 80) {
      n1 = 2;
    }

    if (n1 > 0) {
      usbMIDI.sendNoteOn(40, (pot[2] + 1) * 40, 1);
      Serial.println(" 1");
    }
    if (n1 > 1) {
      usbMIDI.sendNoteOn(40, (pot[1] + 1) * 20, 2);
      Serial.println(" 1");
    }
    n1 = 0;
    if (latchx == 1) {
      scrollx += pot[1];
    }
    if (latchx == 0) {
      scrollx -= pot[1];
    }
    if (scrollx > 10000) {
      latchx = 0;
    }
    if (scrollx < 1) {
      latchx = 1;
    }

    if (latchy == 1) {
      scrolly += pot[2];
    }
    if (latchy == 0) {
      scrolly -= pot[2];
    }
    if (scrolly > 10000) {
      latchy = 0;
    }
    if (scrolly < 1) {
      latchy = 1;
    }
  }

  if (cm - prev[1] > 33) {
    prev[1] = cm;

    pot[0] = analogRead(A0) >> 9 ;
    pot[1] = analogRead(A1) >> 10;
    pot[2] = analogRead(A2) >> 10;
    pot[3] = analogRead(A3) >> 4;
    pot[4] = (pot[3] - 255) * -1;

    usbMIDI.sendControlChange(20, analogRead(A3) >> 5, 4);
    usbMIDI.sendControlChange(21, analogRead(A4) >> 5, 4);
    //usbMIDI.sendControlChange(22, value, channel);


    for ( int xr = 0; xr < 8; xr++) {
      for ( int yc = 0; yc < 8; yc++) {
        int screen = (xr) + ((yc) * 8);
        int scrx = (xr + scrollx) % 24;
        int scry = (yc + scrolly) % 8;
        int mem = ((scrx) + ((scry) * 24));
        //
        //hsv(screen, 0, 0, 0);
        int c2 = (scrollc / 2 + ((xr + 1) * 8) + ((yc + 1) * 16)) % 255;
        hsv(screen, c2 , 255 - (random(20) * 6), (pot[3] / 3) - 2);

        if (text[mem] > 0) {
          int c1 = ((scrollc) + 40 + (text[mem] * 32)) % 255;
          hsv(screen, c1, 220, (pot[4] / 3) - 2);
        }

      }
    }
    leds.show();

  }


  if (cm - prev[0] > 500 && 1 == 0) {
    prev[0] = cm;
    Serial.print(pot[0]);
    Serial.print(" ");
    Serial.println(pot[1]);

  }
}


float fract(float x) {
  return x - int(x);
}

float mix(float a, float b, float t) {
  return a + (b - a) * t;
}

float step(float e, float x) {
  return x < e ? 0.0 : 1.0;
}

byte RedLight;
byte GreenLight;
byte BlueLight;


void hsv(int pixel, int h, int s, int v) {

  if (v < 0) {
    v = 0;
  }


  h = byte(h);
  h = (h * 192) / 256;  // 0..191
  unsigned int i = h / 32;   // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;   // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;  // 0 -> 0xff, 0xff -> 0
  unsigned int fInv = 255 - f;  // 0 -> 0xff, 0xff -> 0
  byte pv = v * sInv / 256;  // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      RedLight = v;
      GreenLight = tv;
      BlueLight = pv;
      break;
    case 1:
      RedLight = qv;
      GreenLight = v;
      BlueLight = pv;
      break;
    case 2:
      RedLight = pv;
      GreenLight = v;
      BlueLight = tv;
      break;
    case 3:
      RedLight = pv;
      GreenLight = qv;
      BlueLight = v;
      break;
    case 4:
      RedLight = tv;
      GreenLight = pv;
      BlueLight = v;
      break;
    case 5:
      RedLight = v;
      GreenLight = pv;
      BlueLight = qv;
      break;
  }
  leds.setPixelColor(pixel, RedLight, GreenLight, BlueLight);

}
