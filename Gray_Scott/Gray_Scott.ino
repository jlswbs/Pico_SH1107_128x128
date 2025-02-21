// Gray-Scott reaction diffusion //

#include "hardware/structs/rosc.h"
#include "sh1107.h"

#define WIDTH   64
#define HEIGHT  64
#define SCR     (WIDTH * HEIGHT)
#define ITER    24

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

  float diffU = 0.16f;
  float diffV = 0.08f;
  float paramF = 0.035f;
  float paramK =  0.06f;
  float *gridU = NULL;  
  float *gridV = NULL;
  float *dU = NULL;
  float *dV = NULL;

void timestep(float F, float K, float diffU, float diffV) {

  for (int j = 1; j < HEIGHT-1; j++) {

    for (int i = 1; i < WIDTH-1; i++) {
            
      float u = gridU[i+j*WIDTH];
      float v = gridV[i+j*WIDTH];          
      float uvv = u * v * v;   
      float lapU = (gridU[(i-1)+j*WIDTH] + gridU[(i+1)+j*WIDTH] + gridU[i+(j-1)*WIDTH] + gridU[i+(j+1)*WIDTH] - 4.0f * u);
      float lapV = (gridV[(i-1)+j*WIDTH] + gridV[(i+1)+j*WIDTH] + gridV[i+(j-1)*WIDTH] + gridV[i+(j+1)*WIDTH] - 4.0f * v);
          
      dU[i+j*WIDTH] = diffU * lapU - uvv + F * (1.0f-u);
      dV[i+j*WIDTH] = diffV * lapV + uvv - (K+F) * v;
      gridU[i+j*WIDTH] += dU[i+j*WIDTH];
      gridV[i+j*WIDTH] += dV[i+j*WIDTH];      
          
    }

  }

}

void setup() {

  seed_random_from_rosc();

  display.begin();
  display.clear();

  gridU = (float*)malloc(4*SCR);
  gridV = (float*)malloc(4*SCR);
  dU = (float*)malloc(4*SCR);
  dV = (float*)malloc(4*SCR);

  memset(gridU, 0, 4*SCR);
  memset(gridV, 0, 4*SCR);
  memset(dU, 0, 4*SCR);
  memset(dV, 0, 4*SCR);

  diffU = randomf(0.0999f, 0.1999f);
  diffV = randomf(0.0749f, 0.0849f);
  paramF = randomf(0.0299f, 0.0399f);
  paramK = randomf(0.0549f, 0.0649f);

  for(int i = 0 ; i < SCR ; i++) {
    
    int px = (i % WIDTH) - (WIDTH / 2);
    int py = (i / HEIGHT) - (WIDTH / 2);

    if(px*px + py*py < 8) {
      gridU[i] = 0.5f * randomf(0.0f, 2.0f);
      gridV[i] = 0.25f * randomf(0.0f, 2.0f);
    } else {
      gridU[i] = 1.0f;
      gridV[i] = 0.0f;
    }
  
  }

}

void loop() {

  for (int k = 0; k < ITER; k++) timestep(paramF, paramK, diffU, diffV);

  for (int y = 0; y < HEIGHT; y++){

    for (int x = 0; x < WIDTH; x++){
  
      uint8_t coll = 255.0f * sinf(gridU[x+y*WIDTH]);
      if(coll < 96) display.drawPixel(2*x, 2*y, WHITE);
      else display.drawPixel(2*x, 2*y, BLACK);
      
    }

  }

  display.display();

}