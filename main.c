#include "mylib.h"
#include "digits.h"

int time_m;
int time_s;
int time_cs;

short digit_to_sprite_tile_index[] = {132, 0, 4, 8, 12, 16, 20, 24, 28, 128};

void displayTime(){
  short time_m_1s = time_m % 10;
  short time_m_10s = time_m / 10;
  short time_s_1s = time_s % 10;
  short time_s_10s = time_s / 10;
  short time_cs_1s = time_cs % 10;
  short time_cs_10s = time_cs / 10;

  char y = 64;
  char x = 8;
  shadowOAM[0].attr0 = ATTR0_AFFINE | y;
  shadowOAM[0].attr1 = ATTR1_MEDIUM | x;
  shadowOAM[0].attr2 = digit_to_sprite_tile_index[time_m_10s];

  shadowOAM[1].attr0 = ATTR0_AFFINE | y;
  shadowOAM[1].attr1 = ATTR1_MEDIUM | (x + 32);
  shadowOAM[1].attr2 = digit_to_sprite_tile_index[time_m_1s];

  shadowOAM[2].attr0 = ATTR0_TALL | ATTR0_AFFINE | y;
  shadowOAM[2].attr1 = ATTR1_MEDIUM | (x + 64);
  shadowOAM[2].attr2 = 136;

  shadowOAM[3].attr0 = ATTR0_AFFINE | y;
  shadowOAM[3].attr1 = ATTR1_MEDIUM | (x + 64 + 16);
  shadowOAM[3].attr2 = digit_to_sprite_tile_index[time_s_10s];

  shadowOAM[4].attr0 = ATTR0_AFFINE | y;
  shadowOAM[4].attr1 = ATTR1_MEDIUM | (x + 64 + 16 + 32);
  shadowOAM[4].attr2 = digit_to_sprite_tile_index[time_s_1s];

  shadowOAM[5].attr0 = ATTR0_TALL | ATTR0_AFFINE | y;
  shadowOAM[5].attr1 = ATTR1_MEDIUM | (x + 64 + 16 + 64);
  shadowOAM[5].attr2 = 138;

  shadowOAM[6].attr0 = ATTR0_AFFINE | y;
  shadowOAM[6].attr1 = ATTR1_MEDIUM | (x + 64 + 16 + 64 + 16);
  shadowOAM[6].attr2 = digit_to_sprite_tile_index[time_cs_10s];

  shadowOAM[7].attr0 = ATTR0_AFFINE | y;
  shadowOAM[7].attr1 = ATTR1_MEDIUM | (x + 64 + 16 + 64 + 16 + 32);
  shadowOAM[7].attr2 = digit_to_sprite_tile_index[time_cs_1s];
}

void interruptHandler(void) {
  REG_IME = 0; //disable interrupts
  // Check which event happened, and do something if you care about it
  if (REG_IF & INT_TM0) {
    time_cs++;
    if (time_cs > 99) time_cs = 0;
  }
  if (REG_IF & INT_TM1) {
    time_s++;
    if (time_s > 59) {
      time_s = time_s - 60;
    }
  }
  if (REG_IF & INT_TM2) {
    time_m++;
  }
  REG_IF = REG_IF;  // Tell GBA that interrupt has
                    // been handled
  REG_IME = 1;      //enable interrupts
}

void enableTimerInterrupts(void) {
  REG_IE = INT_TM0 | INT_TM1 | INT_TM2; // Enable timer interrupts
  // once per cs
  REG_TM0CNT = 0;
  REG_TM0D = 65536 - 164;
  REG_TM0CNT = TM_FREQ_1024 | TIMER_ON | TM_IRQ;

  // once per s
  REG_TM1CNT = 0;
  REG_TM1D = 0;
  REG_TM1CNT = TM_FREQ_256 | TIMER_ON | TM_IRQ;

  // timer 2 ticks when timer 1 overflows and triggers once per minute
  REG_TM2CNT = 0;
  REG_TM2D = 65536 - 60; // 60 seconds in a minute
  REG_TM2CNT = TM_CASCADE | TIMER_ON | TM_IRQ; // cascades from Timer1
}

void setupInterrupts(void) {
  REG_IME = 0; //disable interrupts
  REG_INTERRUPT = interruptHandler; //set int handler
  enableTimerInterrupts();
  REG_IME = 1; //enable interrupts
}


int main(){
  REG_DISPCTL = MODE0 | BG0_ENABLE | SPRITE_ENABLE;

  // set affine matrix to _something_ -- in case I ever get here.
  shadowOAM[0].fill = 256;
  shadowOAM[3].fill = 256;

  // hide all the sprites I don't plan to use;
  hideSprites();

  // start the clock
  setupInterrupts();

  // digits sprites into memory
  DMANow(3, digitsPal, SPRITEPALETTE, digitsPalLen/2);
  DMANow(3, digitsTiles, &CHARBLOCK[4], digitsTilesLen/2);

  //setup timers (1 for each m,s,cs)

  while(1){
    displayTime();
    // button to start
    // button to stop
    // button to reset()
    waitForVBlank();
    DMANow(3, &shadowOAM, OAM, 512);
  }
}