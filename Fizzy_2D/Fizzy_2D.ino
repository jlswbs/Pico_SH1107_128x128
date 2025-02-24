// Fizzy 2D cellular automata //

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

  float *CellVal = NULL;
  uint8_t Calm = 233;
  int CellIndex = 0;

void setup() {

  seed_random_from_rosc();

  display.begin();
  display.clear();

  CellVal = (float*)malloc(4*SCR);

  Calm = 16 + rand()%233;
  for (int i = 0; i < SCR; i++) CellVal[i] = randomf(0.0f, 128.0f);

}

void loop() {

  for (int i = 0; i < WIDTH; i++) {
    
    for (int j = 0; j < HEIGHT; j++) {

      CellIndex = (CellIndex+1)%SCR;

      uint8_t coll = (uint8_t)round(CellVal[CellIndex]*4.7f)%100;
      if(coll< 32) display.drawPixel(i, j, WHITE);
      else display.drawPixel(i, j, BLACK);        

      int below      = (CellIndex+1)%SCR;
      int above      = (CellIndex+SCR-1)%SCR;
      int left       = (CellIndex+SCR-HEIGHT)%SCR;
      int right      = (CellIndex+HEIGHT)%SCR;
      int aboveright = ((CellIndex-1) + HEIGHT + SCR)%SCR;
      int aboveleft  = ((CellIndex-1) - HEIGHT + SCR)%SCR;
      int belowright = ((CellIndex+1) + HEIGHT + SCR)%SCR;
      int belowleft  = ((CellIndex+1) - HEIGHT + SCR)%SCR;

      float NeighbourMix = powf((CellVal[left]*CellVal[right]*CellVal[above]*CellVal[below]*CellVal[belowleft]*CellVal[belowright]*CellVal[aboveleft]*CellVal[aboveright]),0.125f);
      CellVal[CellIndex] = fmod((sqrtf(CellVal[CellIndex]*NeighbourMix))+0.5f, Calm);

    }

  }

  display.display();

}