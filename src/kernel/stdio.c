#include "stdio.h"

#include <stdint.h>

const unsigned SCREEN_WIDTH = 80;
const unsigned SCREEN_HEIGHT = 25;
const uint8_t DEFAULT_COLOR;

uint8_t* g_ScreenBuffer = (uint8_t*)0xB8000;
int g_ScreenX = 0, g_ScreenY = 0;

void putchr(int x, int y, char c) {
    g_ScreenBuffer[(y * SCREEN_WIDTH + x) * 2] = c;
}

void putcolor(int x, int y, uint8_t color) {
    g_ScreenBuffer[(y * SCREEN_WIDTH + x) * 2 + 1] = color;
}

void putc(char c) {
    switch (c) {
        case '\n':
            g_ScreenY++;
            g_ScreenX = 0;
            break;
        case '\r':
            g_ScreenX = 0;
            break;
        default:
            putchr(g_ScreenX, g_ScreenY, c);
            g_ScreenX++;
    }

    if (g_ScreenX >= SCREEN_WIDTH) {
        g_ScreenX = 0;
        g_ScreenY++;
    }
}

void puts(char* str) {
    while (*str) {
        putc(*str);
        str++;
    }
}