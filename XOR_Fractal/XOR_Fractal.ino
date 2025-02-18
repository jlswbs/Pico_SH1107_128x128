// XOR fractal //

#include "sh1107.h"

#define WIDTH   128
#define HEIGHT  128

SH1107 display;

int cnt;

void setup() {

  display.begin();
  display.clear();

}

void loop() {

  for (int x = 0; x < WIDTH; x++) {

    for (int y = 0; y < HEIGHT; y++) {

      bool coll = ((x ^ y) + cnt) % 20;
      display.drawPixel(x, y, !coll);

    }

  }

  display.display();

  cnt++;

}