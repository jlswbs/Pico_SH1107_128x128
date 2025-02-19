// Simplest 1D cellular automaton // 

#include "sh1107.h"

#define WIDTH   128
#define HEIGHT  128

SH1107 display;

  bool state[WIDTH];
  bool k;

void setup() {

  display.begin();
  display.clear();

  state[0] = 1;

}

void loop() {

  for(int y=0; y<HEIGHT; y++) {

    for(int x=0; x<WIDTH; x++) {
      
      k = k ^ state[x];
      state[x] = k;

      display.drawPixel(x, y, state[x]);
 
    }

  }

  display.display();

}