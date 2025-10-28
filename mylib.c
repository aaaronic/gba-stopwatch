#include "mylib.h"

// The start of the video memory
unsigned short *videoBuffer = (unsigned short *)0x6000000;

// The shadowOAM
OBJ_ATTR shadowOAM[128];


// Set up and begin a DMA transfer
void DMANow(int channel, volatile const void *src, volatile void *dst, unsigned int cnt) {
    DMA[channel].cnt = 0;
    DMA[channel].src = src;
    DMA[channel].dst = dst;
    DMA[channel].cnt = cnt | DMA_ON;
}

void hideSprites() {
    for (int i = 0; i < 128; i++) {
        shadowOAM[i].attr0 = ATTR0_HIDE;
    }
}