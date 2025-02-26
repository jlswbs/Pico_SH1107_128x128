// Amoeba patterns //

#include "hardware/structs/rosc.h"
#include "sh1107.h"

#define WIDTH   128
#define HEIGHT  128
#define SCR     (WIDTH * HEIGHT)
#define NUMS    96

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

  float *p = NULL;
  float *v = NULL;
  float *a = NULL;

void setup() {

  seed_random_from_rosc();

  display.begin();
  display.clear();

  p = (float*)malloc(4*SCR);
  v = (float*)malloc(4*SCR);
  a = (float*)malloc(4*SCR);

  memset(p, 0, 4*SCR);
  memset(v, 0, 4*SCR);
  memset(a, 0, 4*SCR);

  for (int i = 0; i < NUMS; i++) v[(1+rand()%(WIDTH-2))+(1+rand()%(HEIGHT-2))*WIDTH] = randomf(0.0f, 1.0f);

}

void loop() {

  for (int y = 1; y < HEIGHT-1; y++) {

    for (int x = 1; x < WIDTH-1; x++) {

      a[x+y*WIDTH] = (v[(x-1)+y*WIDTH] + v[(x+1)+y*WIDTH] + v[x+(y-1)*WIDTH] + v[x+(y+1)*WIDTH]) * 0.25f - v[x+y*WIDTH];

    }

  }

  for (int j=0; j<HEIGHT; j++) {
    
    for (int i=0; i<WIDTH; i++) {
      
      v[i+j*WIDTH] += a[i+j*WIDTH];
      p[i+j*WIDTH] += v[i+j*WIDTH];
      uint8_t coll = 255.0f * logf(p[i+j*WIDTH]);    
      if(coll < 48) display.drawPixel(i, j, BLACK);
      else display.drawPixel(i, j, WHITE);

    }
  
  }

  display.display();

}