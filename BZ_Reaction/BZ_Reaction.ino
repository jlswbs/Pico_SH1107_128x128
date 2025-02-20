// Belousov-Zabotinsky reaction //

#include "hardware/structs/rosc.h"
#include "sh1107.h"

#define WIDTH   64
#define HEIGHT  64
#define SCR     (WIDTH * HEIGHT)
#define SCL     2

SH1107 display;

float randomf(float minf, float maxf) {return minf + (rand()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}

static inline void seed_random_from_rosc(){
  
  uint32_t random = 0;
  uint32_t random_bit;
  volatile uint32_t *rnd_reg = (uint32_t *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

  for (int k = 0; k < 32; k++) {
    while (1) {
      random_bit = (*rnd_reg) & 1;
      if (random_bit != ((*rnd_reg) & 1)) break;
    }

    random = (random << 1) | random_bit;
  }

  srand(random);

}

  float *a = NULL;
  float *b = NULL;
  float *c = NULL;
  float adjust = 1.2f;
  bool p = 0, q = 1;

void setup() {

  seed_random_from_rosc();

  display.begin();
  display.clear();

  a = (float*)malloc(4*SCR*SCL);
  b = (float*)malloc(4*SCR*SCL);
  c = (float*)malloc(4*SCR*SCL);

  memset(a, 0, 4*SCR*SCL);
  memset(b, 0, 4*SCR*SCL);
  memset(c, 0, 4*SCR*SCL);

  adjust = randomf(0.75f, 1.35f);

  for(int i=0; i<SCR; i++){

    a[i] = randomf(0.0f, 1.0f);
    b[i] = randomf(0.0f, 1.0f);
    c[i] = randomf(0.0f, 1.0f);
  
  }

}

void loop() {

  for (int y = 0; y < HEIGHT; y++){

    for (int x = 0; x < WIDTH; x++){
  
      float c_a = 0;
      float c_b = 0;
      float c_c = 0;
    
      for (int i = x - 1; i <= x+1; i++) {

        for (int j = y - 1; j <= y+1; j++) {

          c_a += a[(((i+WIDTH)%WIDTH)+((j+HEIGHT)%HEIGHT)*WIDTH)+SCR*p];
          c_b += b[(((i+WIDTH)%WIDTH)+((j+HEIGHT)%HEIGHT)*WIDTH)+SCR*p];
          c_c += c[(((i+WIDTH)%WIDTH)+((j+HEIGHT)%HEIGHT)*WIDTH)+SCR*p];
        }
      }
      
      c_a /= 9.0f;
      c_b /= 9.0f;
      c_c /= 9.0f;

      a[(x+y*WIDTH)+SCR*q] = constrain(c_a + c_a * (adjust * c_b - c_c), 0.0f, 1.0f);
      b[(x+y*WIDTH)+SCR*q] = constrain(c_b + c_b * (c_c - adjust * c_a), 0.0f, 1.0f);
      c[(x+y*WIDTH)+SCR*q] = constrain(c_c + c_c * (c_a - c_b), 0.0f, 1.0f);      
    
      uint8_t coll = 255.0f * a[(x+y*WIDTH)+SCR*q];
      if(coll < 128) display.drawPixel(2*x, 2*y, BLACK);
      else display.drawPixel(2*x, 2*y, WHITE);
      
    }
  }

  if (p == 0) { p = 1; q = 0; } else { p = 0; q = 1; }

  display.display();

}