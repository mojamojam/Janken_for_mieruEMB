volatile char *e_vram = (char*)0x900000;
volatile int  *e_time = (int *)0x80010c;
volatile char *e_gp1  = (char*)0x8001f0;
volatile char *e_gp2  = (char*)0x8001f1;
volatile char *e_sw1  = (char*)0x8001fc;
volatile char *e_sw2  = (char*)0x8001fd;
volatile char *e_sw3  = (char*)0x8001fe;
volatile char *e_gin  = (char*)0x8001ff;

#define C_WIDTH 32;
#define C_HEIGHT 32;
#include "cfont.h"

static unsigned int seed = 1;

int my_rand() {
    seed = (seed + *e_time) * 1103515245+12345;
    return (unsigned int)(seed / 65536) % 32768;
}

void mylib_putPic(int x, int y, int width, int height, int num) {
    int i,j;
    for(i=0; i<height; i++)
        for(j=0; j<width; j++)
            e_vram[(x+j) + (y+i)*128] = e_character[num][i][j];
}

void mylib_putc(int x, int y, char c, int color) {
    int i, j;
    for(i=0; i<16; i++) {
        for(j=0; j<8; j++) {
            if(e_char[(int)(c-'A')][i][j])
                e_vram[(x+j)+(y+i)*128] = color;
        }
    }
}

void mylib_putnum(int x, int y, int num, int color) {
    int i, j;
    for(i=0; i<16; i++) {
        for(j=0; j<8; j++) {
            if(e_number[num][i][j]) e_vram[(x+j)+(y+i)*128] = color;
        }
    }
}

void mylib_clear(int color) {
    int i;
    for(i=0; i<128*128; i++) e_vram[i] = color;
}

void clearxy(int x, int y, int width, int height) {
    int i, j;
    for(i=0; i<height; i++){
        for(j=0; j<width; j++){
            e_vram[(x+j)+(y+i)*128] = 0;
        }
    }
}

void putJudge(int result) {
    int x = 0;
    int y = 16;
    clearxy(x, y, 32, 16);
    if (result == 0) {
        //DRAW
        mylib_putc(x, y, 'D', 7); x = x+8;
        mylib_putc(x, y, 'R', 7); x = x+8;
        mylib_putc(x, y, 'A', 7); x = x+8;
        mylib_putc(x, y, 'W', 7);
    } else if (result == 1) {
        //LOSE
        mylib_putc(x, y, 'L', 7); x = x+8;
        mylib_putc(x, y, 'O', 7); x = x+8;
        mylib_putc(x, y, 'S', 7); x = x+8;
        mylib_putc(x, y, 'E', 7);
    } else {
        //WIN
        mylib_putc(x, y, 'W', 7); x = x+8;
        mylib_putc(x, y, 'I', 7); x = x+8;
        mylib_putc(x, y, 'N', 7);
    }
}

int judge(int hand) {
    int cpuHand = my_rand() % 3;
    mylib_putPic(50, 40, 32, 32, cpuHand);
    int result = (hand - cpuHand + 3) % 3;
    putJudge(result);
    if (result == 0) {
        return 0;
    } else if (result == 2) {
        return 2;
    } else {
        return 1;
    }
}

void mylib_msleep(unsigned int tm) {
    unsigned int end = (unsigned int) *e_time + tm;
    while (*e_time < end);
}

void putWinCount(int winCount) {
    int x = 104;
    int y = 0;
    clearxy(x, y, 24, 16);
    int num[3] = {};
    num[0] = winCount % 10; winCount = winCount / 10;
    num[1] = winCount % 10; winCount = winCount / 10;
    num[2] = winCount % 10; winCount = winCount / 10;
    mylib_putnum(x, y, num[2], 7); x = x+8;
    mylib_putnum(x, y, num[1], 7); x = x+8;
    mylib_putnum(x, y, num[0], 7);
}

void putMessage() {
    int x = 0;
    int y = 0;
    mylib_putc(x, y, 'P', 7); x = x+8;
    mylib_putc(x, y, 'U', 7); x = x+8;
    mylib_putc(x, y, 'S', 7); x = x+8;
    mylib_putc(x, y, 'H', 7); x = x+16;

    mylib_putc(x, y, 'B', 7); x = x+8;
    mylib_putc(x, y, 'U', 7); x = x+8;
    mylib_putc(x, y, 'T', 7); x = x+8;
    mylib_putc(x, y, 'T', 7); x = x+8;
    mylib_putc(x, y, 'O', 7); x = x+8;
    mylib_putc(x, y, 'N', 7);
}
int janken() {
    int winCount = 0;
    int result = -1;
    putMessage();
    mylib_putPic(10, 90, 32, 32, 0);
    mylib_putPic(50, 90, 32, 32, 1);
    mylib_putPic(90, 90, 32, 32, 2);
    putWinCount(winCount);
    while(1) {
        if (*e_sw1 == 0 && *e_sw2 != 0 && *e_sw3 != 0) {
            while(*e_sw1 == 0){}
            result = judge(0);
        } else if (*e_sw2 == 0 && *e_sw1 != 0 && *e_sw3 != 0) {
            while(*e_sw2 == 0){}
            result = judge(1);
        } else if (*e_sw3 == 0 && *e_sw1 != 0 && *e_sw2 != 0) {
            while(*e_sw3 == 0){}
            result = judge(2);
        }

        if (result == 2) {
            winCount++;
            putWinCount(winCount);
            result = -1;
        } else if(result == 1) {
            winCount = 0;
            putWinCount(winCount);
            result = -1;
        }
        mylib_msleep(10);
    }
}

int selectHand(int rock, int siz, int par) {
    int hand = -1;
    if (*e_sw1 == 0 && *e_sw2 != 0 && *e_sw3 != 0) {
        while(*e_sw1 == 0){}
        hand = 0;
    } else if (*e_sw2 == 0 && *e_sw1 != 0 && *e_sw3 != 0) {
        while(*e_sw2 == 0){}
        hand = 1;
    } else if (*e_sw3 == 0 && *e_sw1 != 0 && *e_sw2 != 0) {
        while(*e_sw3 == 0){}
        hand = 2;
    }

    if (hand == 0 && rock == 1) {
        return hand;
    } else if (hand == 1 && siz == 1) {
        return hand;
    } else if (hand == 2 && par == 1) {
        return hand;
    }

}


int main() {
    mylib_clear(0);
    janken();
    return 0;
}