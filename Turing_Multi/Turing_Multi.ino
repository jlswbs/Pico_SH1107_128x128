// Multi scale Turing patterns //

#include "hardware/structs/rosc.h"
#include "sh1107.h"

#define WIDTH   85
#define HEIGHT  85
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

  int level, levels, radius;
  int blurlevels;
  float base;
  float stepScale;
  float stepOffset;
  float blurFactor;
  uint8_t *radii = NULL;
  float *stepSizes = NULL;
  float *grid = NULL;
  float *blurBuffer = NULL;
  float *bestVariation = NULL;
  uint8_t *bestLevel = NULL;
  bool *direction = NULL;
  float *activator = NULL;
  float *inhibitor = NULL;
  float *swap = NULL;

void setup() {

  seed_random_from_rosc();

  display.begin();
  display.clear();

  bestLevel = (uint8_t*)malloc(4*SCR);
  grid = (float*)malloc(4*SCR);
  blurBuffer = (float*)malloc(4*SCR);
  bestVariation = (float*)malloc(4*SCR);
  activator = (float*)malloc(4*SCR);
  inhibitor = (float*)malloc(4*SCR);
  swap = (float*)malloc(4*SCR);
  direction = (bool*)malloc(4*SCR);
  stepSizes = (float*)malloc(WIDTH);
  radii = (uint8_t*)malloc(WIDTH);

  base = randomf(1.45f, 1.85f);
  stepScale = randomf(0.01f, 0.2f);
  stepOffset = randomf(0.01f, 0.4f);
  blurFactor = randomf(0.5f, 1.0f);

  levels = (int) (log(fmax(WIDTH,HEIGHT)) / logf(base)) - 1.0f;
  blurlevels = (int) fmax(0, (levels+1) * blurFactor - 0.5f);

  for (int i = 0; i < levels; i++) {
    int radius = (int)powf(base, i);
    radii[i] = radius;
    stepSizes[i] = logf(radius) * stepScale + stepOffset;  
  }

  for (int i = 0; i < SCR; i++) grid[i] = randomf(-1.0f, 1.0f);
  
}

void loop() {
   
  memcpy(activator, grid, 4*SCR);

  for (level = 0; level < levels - 1; level++) {

    int radius = radii[level];

    if(level <= blurlevels){
        
      for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
          int t = y * WIDTH + x;
          if (y == 0 && x == 0) blurBuffer[t] = activator[t];
          else if (y == 0) blurBuffer[t] = blurBuffer[t - 1] + activator[t];
          else if (x == 0) blurBuffer[t] = blurBuffer[t - WIDTH] + activator[t];
          else blurBuffer[t] = blurBuffer[t - 1] + blurBuffer[t - WIDTH] - blurBuffer[t - WIDTH - 1] + activator[t];
        }
      }
    }

    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        int minx = max(0, x - radius);
        int maxx = min(x + radius, WIDTH - 1);
        int miny = max(0, y - radius);
        int maxy = min(y + radius, HEIGHT - 1);
        int area = (maxx - minx) * (maxy - miny);

        int nw = miny * WIDTH + minx;
        int ne = miny * WIDTH + maxx;
        int sw = maxy * WIDTH + minx;
        int se = maxy * WIDTH + maxx;

        int t = y * WIDTH + x;
        inhibitor[t] = (blurBuffer[se] - blurBuffer[sw] - blurBuffer[ne] + blurBuffer[nw]) / area;
      }
    }
        
    for (int i = 0; i < SCR; i++) {
      float variation = fabs(activator[i] - inhibitor[i]);
      if (level == 0 || variation < bestVariation[i]) {
        bestVariation[i] = variation;
        bestLevel[i] = level;
        direction[i] = activator[i] > inhibitor[i];
      }
    }

    if(level==0) {
      memcpy(activator, inhibitor, 4*SCR);
    } else {
      memcpy(swap, activator, 4*SCR);
      memcpy(activator, inhibitor, 4*SCR);
      memcpy(inhibitor, swap, 4*SCR);
    }
            
  }

  float smallest = MAXFLOAT;
  float largest = -MAXFLOAT;

  for (int i = 0; i < SCR; i++) {
    float curStep = stepSizes[bestLevel[i]];
    if (direction[i]) {
      grid[i] += curStep;  
    } else {
      grid[i] -= curStep;         
    }
    smallest = fmin(smallest, grid[i]);
    largest = fmax(largest, grid[i]);   
  }

  float range = (largest - smallest) / 2.0f;

  for (int y = 0; y < HEIGHT; y++){

    for (int x = 0; x < WIDTH; x++){

      grid[x+y*WIDTH] = ((grid[x+y*WIDTH] - smallest) / range) - 1.0f;
      uint8_t coll = 128 + (127.0f * grid[x+y*WIDTH]);
      if(coll < 96) display.drawPixel(1.5f*x, 1.5f*y, WHITE);
      else display.drawPixel(1.5f*x, 1.5f*y, BLACK);
      
    }

  }

  display.display();

}