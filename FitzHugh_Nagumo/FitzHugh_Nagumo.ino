// FitzHugh-Nagumo reaction diffusion //

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

  float reactionRate = 0.2f;
  float diffusionRate = 0.01f;
  float kRate = 0.4f;
  float fRate = 0.09f;  
  float *gridU = NULL;  
  float *gridV = NULL;
  float *gridNext = NULL;
  float *diffRateUYarr = NULL;
  float *diffRateUXarr = NULL;
  float *farr = NULL;
  float *karr = NULL;
  float *temp = NULL;

void diffusionV(){
  
  for(int i=0;i<WIDTH;++i){
    for(int j=0;j<HEIGHT;++j){

      gridNext[i+j*WIDTH] = gridV[i+j*WIDTH]+diffusionRate*4.0f*(gridV[((i-1+WIDTH)%WIDTH)+j*WIDTH]+gridV[((i+1)%WIDTH)+j*WIDTH]+gridV[i+((j-1+HEIGHT)%HEIGHT)*WIDTH]+gridV[i+((j+1)%HEIGHT)*WIDTH]-4.0f*gridV[i+j*WIDTH]);
    
    }
  }
  
  memcpy(temp, gridV, 4*SCR);
  memcpy(gridV, gridNext, 4*SCR); 
  memcpy(gridNext, temp, 4*SCR);
  
}

void diffusionU(){
  
  for(int i=0;i<WIDTH;++i){
    for(int j=0;j<HEIGHT;++j){

      gridNext[i+j*WIDTH] = gridU[i+j*WIDTH]+4.0f*(diffRateUXarr[i+j*WIDTH]*(gridU[((i-1+WIDTH)%WIDTH)+j*WIDTH]+gridU[((i+1)%WIDTH)+j*WIDTH]-2.0f*gridU[i+j*WIDTH])+diffRateUYarr[i+j*WIDTH]*(gridU[i+((j-1+HEIGHT)%HEIGHT)*WIDTH]+gridU[i+((j+1)%HEIGHT)*WIDTH]-2.0f*gridU[i+j*WIDTH]));
    
    }
  }
  
  memcpy(temp, gridU, 4*SCR);
  memcpy(gridU, gridNext, 4*SCR); 
  memcpy(gridNext, temp, 4*SCR);
  
}

void reaction(){

  for(int i=0;i<SCR;++i){

    gridU[i] = gridU[i]+4.0f*(reactionRate*(gridU[i]-gridU[i]*gridU[i]*gridU[i]-gridV[i]+karr[i]));
    gridV[i] = gridV[i]+4.0f*(reactionRate*farr[i]*(gridU[i]-gridV[i]));
    
  }

}

void setupF(){
  
  for(int i=0;i<WIDTH;++i){
  
    for(int j=0;j<HEIGHT;++j) farr[i+j*WIDTH] = 0.01f + i * fRate / WIDTH;
  
  }

}

void setupK(){

  for(int i=0;i<WIDTH;++i){
  
    for(int j=0;j<HEIGHT;++j) karr[i+j*WIDTH] = 0.06f + j * kRate / HEIGHT;
    
  }

}

void setup() {

  seed_random_from_rosc();

  display.begin();
  display.clear();

  gridU = (float*)malloc(4*SCR);
  gridV = (float*)malloc(4*SCR);
  gridNext = (float*)malloc(4*SCR);
  diffRateUYarr = (float*)malloc(4*SCR);
  diffRateUXarr = (float*)malloc(4*SCR);
  farr = (float*)malloc(4*SCR);
  karr = (float*)malloc(4*SCR);
  temp = (float*)malloc(4*SCR);

  diffusionRate = randomf(0.01f, 0.05f);
  kRate = randomf(0.1f, 0.5f);
  fRate = randomf(0.04f, 0.09f);
  
  for(int i=0;i<SCR;++i){
      
    gridU[i] = 0.5f + randomf(-0.01f, 0.01f);
    gridV[i] = 0.25f + randomf(-0.01f, 0.01f);

  }

  setupF();
  setupK();

  for(int i=0;i<SCR;++i){
    
    diffRateUYarr[i] = randomf(0.03f, 0.05f);
    diffRateUXarr[i] = randomf(0.03f, 0.05f);
      
  }

}

void loop() {

  diffusionU();
  diffusionV();
  reaction();

  for (int y = 0; y < HEIGHT; y++){

    for (int x = 0; x < WIDTH; x++){
  
      uint8_t coll = 255.0f * gridU[x+y*WIDTH];
      if(coll < 96) display.drawPixel(1.5f*x, 1.5f*y, WHITE);
      else display.drawPixel(1.5f*x, 1.5f*y, BLACK);
      
    }

  }

  display.display();

}