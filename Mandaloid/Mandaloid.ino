// Mandaloid patterns //

#include "hardware/structs/rosc.h"
#include "sh1107.h"

#define WIDTH   128
#define HEIGHT  128
#define SCR     (WIDTH * HEIGHT)

SH1107 display;

float randomf(float minf, float maxf) {return minf + (rand()%(1UL << 31)) * (maxf - minf) / (1UL << 31);}

float sigmoid(float v) {

    constexpr float c1 = 0.03138777f;
    constexpr float c2 = 0.276281267f;
    constexpr float c_log2f = 1.442695022f;
    v *= c_log2f * 0.5f;
    int intPart = (int)v;
    float x = (v - intPart);
    float xx = x * x;
    float v1 = c_log2f + c2 * xx;
    float v2 = x + xx * c1 * x;
    float v3 = (v2 + v1);
    *((int*)&v3) += intPart << 24;
    float v4 = v2 - v1;
    float res = v3 / (v3 - v4);
    return res;

}

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

  a = randomf(-0.25f, 0.25f);
  float seed = randomf(-TWO_PI, TWO_PI);

  v[(WIDTH/2)+(HEIGHT/2)*WIDTH] = seed;
  v[(WIDTH/2)+((HEIGHT/2)-1)*WIDTH] = seed;
  v[((WIDTH/2)-1)+((HEIGHT/2)-1)*WIDTH] = seed;
  v[((WIDTH/2)-1)+(HEIGHT/2)*WIDTH] = seed;

}

void loop() {

  for (int y = 1; y < HEIGHT-1; y++) {

    for (int x = 1; x < WIDTH-1; x++) {

      v[x+y*WIDTH] += sigmoid(p[(x+1)+(y+1)*WIDTH] + p[(x-1)+(y-1)*WIDTH] + p[(x+1)+(y-1)*WIDTH] + p[(x-1)+(y+1)*WIDTH]) * a - p[x+y*WIDTH];

    }

  }

  for (int j=0; j<HEIGHT; j++) {
    
    for (int i=0; i<WIDTH; i++) {
      
      p[i+j*WIDTH] += v[i+j*WIDTH];
      uint8_t coll = 255 - (255.0f * p[i+j*WIDTH]);    
      if(coll < 128) display.drawPixel(i, j, WHITE);
      else display.drawPixel(i, j, BLACK);

    }
  
  }

  display.display();

}