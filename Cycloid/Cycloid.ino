// 2D Cycloid patterns //

#include "hardware/structs/rosc.h"
#include "sh1107.h"

#define WIDTH   128
#define HEIGHT  128
#define SCR     (WIDTH * HEIGHT)

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

  float a = 0.25f;
  float *p = NULL;
  float *v = NULL;


void setup() {

  seed_random_from_rosc();

  display.begin();
  display.clear();

  p = (float*)malloc(4*SCR);
  v = (float*)malloc(4*SCR);

  memset(p, 0, 4*SCR);
  memset(v, 0, 4*SCR);

  a = randomf(0.23f, 0.27f);
  float seed = randomf(0.0f, PI);

  v[(WIDTH/2)+(HEIGHT/2)*WIDTH] = seed;
  v[(WIDTH/2)+((HEIGHT/2)-1)*WIDTH] = seed;
  v[((WIDTH/2)-1)+((HEIGHT/2)-1)*WIDTH] = seed;
  v[((WIDTH/2)-1)+(HEIGHT/2)*WIDTH] = seed;

}

void loop() {

  for (int y = 1; y < HEIGHT-1; y++) {

    for (int x = 1; x < WIDTH-1; x++) {

      p[x+y*WIDTH] = (v[(x+1)+(y+1)*WIDTH] + v[(x-1)+(y-1)*WIDTH] + v[(x+1)+(y-1)*WIDTH] + v[(x-1)+(y+1)*WIDTH]) * a - v[x+y*WIDTH];

    }

  }

  for (int j=0; j<HEIGHT; j++) {
    
    for (int i=0; i<WIDTH; i++) {
      
      v[i+j*WIDTH] += p[i+j*WIDTH];
      uint8_t coll = 255 - (255.0f * logf(v[i+j*WIDTH]));    
      if(coll < 128) display.drawPixel(i, j, WHITE);
      else display.drawPixel(i, j, BLACK);

    }
  
  }

  display.display();

}