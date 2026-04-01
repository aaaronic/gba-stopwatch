#include "mylib.h"
#include "digits.h"

int time_m;
int time_s;
int time_cs;

u16 show_cs = 1;
u16 pal0_color = COLOR(5,6,7);

short digit_to_sprite_tile_index[] = {132, 0, 4, 8, 12, 16, 20, 24, 28, 128};
/* !!!!! For learning purposes only -- DO NOT COPY+PASTE FROM THIS FILE !!!!! */

void displayTime(){
  u16 affinity = time_m >= 99 && time_s >= 59 ? 0 : ATTR0_AFFINE;

  short time_m_1s = time_m % 10;
  short time_m_10s = time_m / 10;
  short time_s_1s = time_s % 10;
  short time_s_10s = time_s / 10;
  short time_cs_1s = time_cs % 10;
  short time_cs_10s = time_cs / 10;

  char y = 64;
  char x = show_cs ? 8 : 32+16;
  shadowOAM[0].attr0 = affinity | y;
  shadowOAM[0].attr1 = ATTR1_MEDIUM | x | (4 << 9);  // use affine matrix 4;
  shadowOAM[0].attr2 = digit_to_sprite_tile_index[time_m_10s];

  shadowOAM[1].attr0 = affinity | y;
  shadowOAM[1].attr1 = ATTR1_MEDIUM | (x + 32) | (3 << 9);  // use affine matrix 3
  shadowOAM[1].attr2 = digit_to_sprite_tile_index[time_m_1s];

  shadowOAM[2].attr0 = ATTR0_TALL | y;
  shadowOAM[2].attr1 = ATTR1_MEDIUM | (x + 64);
  shadowOAM[2].attr2 = 136;

  shadowOAM[3].attr0 = affinity | y;
  shadowOAM[3].attr1 = ATTR1_MEDIUM | (x + 64 + 16) | (2 << 9);  // use affine matrix 2
  shadowOAM[3].attr2 = digit_to_sprite_tile_index[time_s_10s];

  shadowOAM[4].attr0 = affinity | y;
  shadowOAM[4].attr1 = ATTR1_MEDIUM | (x + 64 + 16 + 32) | (1 << 9);  // use affine matrix 1
  shadowOAM[4].attr2 = digit_to_sprite_tile_index[time_s_1s];

  if (show_cs) {
    shadowOAM[5].attr0 = ATTR0_TALL | y;
    shadowOAM[5].attr1 = ATTR1_MEDIUM | (x + 64 + 16 + 64);
    shadowOAM[5].attr2 = 138;

    shadowOAM[6].attr0 = y;
    shadowOAM[6].attr1 = ATTR1_MEDIUM | (x + 64 + 16 + 64 + 16);
    shadowOAM[6].attr2 = digit_to_sprite_tile_index[time_cs_10s];

    shadowOAM[7].attr0 = y;
    shadowOAM[7].attr1 = ATTR1_MEDIUM | (x + 64 + 16 + 64 + 16 + 32);
    shadowOAM[7].attr2 = digit_to_sprite_tile_index[time_cs_1s];
  }

  if (time_cs_10s > 7) {
    shadowOAM[7].fill -= 8;  // AFFINE 1 y
  } else {
    shadowOAM[7].fill = 256;
  }

  if (time_s_1s == 9 && time_cs_10s > 7) {
    shadowOAM[11].fill -= 8;  // AFFINE 2 y
  } else {
    shadowOAM[11].fill = 256;
  }

  if (time_s_1s == 9 && time_s_10s == 5 && time_cs_10s > 7) {
    shadowOAM[15].fill -= 8;  // AFFINE 3 y
  } else {
    shadowOAM[15].fill = 256;
  }

  if (time_s_1s == 9 && time_s_10s == 5 && time_m_1s == 9 && time_cs_10s > 7) {
    shadowOAM[19].fill -= 8;  // AFFINE 4 y
  } else {
    shadowOAM[19].fill = 256;
  }
}

/* !!!!! For learning purposes only -- DO NOT COPY+PASTE FROM THIS FILE !!!!! */
void interruptHandler(void) {
  REG_IME = 0; //disable interrupts

  if (REG_IF & INT_HBLANK) { // gotta go fast special case!
    PALETTE[0] = (BUTTON_HELD(BUTTON_START) && (SCANLINECOUNTER & 16)) ? pal0_color ^ COLOR(31,31,31) : pal0_color;

    REG_IF = INT_HBLANK; // handled just the one!
    REG_IME = 1;
    return; // exit early so we don't take too long
  }

  // Check which event happened, and do something if you care about it
  if (REG_IF & INT_TM0) {
    time_cs++;
    if (time_cs < 0) time_cs += 100;
    REG_IF = INT_TM0;
  }
  if (REG_IF & INT_TM1) {
    time_s++;
    time_cs=0;  // time_cs is inherently inaccurate, so this fixes some flipping glitches.
    if (time_s >= 60) {
      time_s -= 60;
      time_m++;
    }
    if (time_m > 99) {
      PALETTE[0] = RED;
      time_m = 99;
      time_s = 59;
      time_cs = 99;
      REG_TM0CNT = 0;
      REG_TM1CNT = 0;
    }
    REG_IF = INT_TM1;
  }

  if (REG_IF & INT_VBLANK) {
    DMANow(3, shadowOAM, OAM, sizeof(shadowOAM)/2);
    REG_IF = INT_VBLANK;
  }

  REG_IME = 1;      //re-enable interrupts
}

/* !!!!! For learning purposes only -- DO NOT COPY+PASTE FROM THIS FILE !!!!! */
void enableTimerInterrupts(void) {
  REG_IE |= INT_TM0 | INT_TM1; // Enable timer interrupts
  // once per cs
  REG_TM0CNT = 0;
  REG_TM0D = 65536 - 2621;
  REG_TM0CNT = TM_FREQ_64 | TIMER_ON | TM_IRQ;

  // once per s
  REG_TM1CNT = 0;
  REG_TM1D = 0;
  REG_TM1CNT = TM_FREQ_256 | TIMER_ON | TM_IRQ;
}

void enableVBlankInterrupt(void){
  REG_IE |= INT_VBLANK;
  REG_DISPSTAT |= DISPSTAT_VBLANK_INT_ENABLE;
}

void enableHBlankInterrupt(void){
  REG_IE |= INT_HBLANK;
  REG_DISPSTAT |= DISPSTAT_HBLANK_INT_ENABLE;
}

void setupInterrupts(void) {
  REG_IME = 0; //disable interrupts
  REG_INTERRUPT = interruptHandler; //set int handler
  enableTimerInterrupts();
  enableVBlankInterrupt();
  enableHBlankInterrupt();
  REG_IME = 1; //enable interrupts
}

/* !!!!! For learning purposes only -- DO NOT COPY+PASTE FROM THIS FILE !!!!! */
int main(){
  REG_DISPCTL = MODE0 | BG0_ENABLE | SPRITE_ENABLE;

  // set affine matrix to the identity matrix initially.
  shadowOAM[0].fill = 256;  // AFFINE 0
  shadowOAM[3].fill = 256;

  shadowOAM[4].fill = 256;  // AFFINE 1
  shadowOAM[7].fill = 256;

  shadowOAM[8].fill = 256;  // AFFINE 2
  shadowOAM[11].fill = 256;

  shadowOAM[12].fill = 256;  // AFFINE 3
  shadowOAM[15].fill = 256;

  shadowOAM[16].fill = 256;  // AFFINE 4
  shadowOAM[19].fill = 256;

  // hide all the sprites I don't plan to use;
  hideSprites();

  // start the clock
  setupInterrupts();

  // digits sprites into memory
  DMANow(3, digitsPal, SPRITEPALETTE, digitsPalLen/2);
  DMANow(3, digitsTiles, &CHARBLOCK[4], digitsTilesLen/2);

  while(1){
    displayTime();
    __asm__("swi 0x02 << 16");  // low power CPU until next interrupt
  }
}