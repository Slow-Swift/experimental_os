#include "vga_text.h"

#include "io.h"
#include <stdbool.h>

const unsigned SCREEN_WIDTH = 80;
const unsigned SCREEN_HEIGHT = 25;
const uint8_t DEFAULT_COLOR = 0x7;

static uint8_t* screen_buffer = (uint8_t*)0xB8000;
static int cursor_x = 0, cursor_y = 0;

/**
 * Set the character at x, y to c and reset to the default color
*/
static void set_char(int x, int y, char c) 
{
    screen_buffer[2 * (y * SCREEN_WIDTH + x)] = c;
    screen_buffer[2 * (y * SCREEN_WIDTH + x) + 1] = DEFAULT_COLOR;
}

static char get_char(int x, int y) 
{
    return screen_buffer[2 * (y * SCREEN_WIDTH + x)];
}

/**
 * Update the VGA cursor to the current x, y position
*/
static void update_cursor() 
{
    int pos = cursor_y * SCREEN_WIDTH + cursor_x;

    out_byte(0x3D4, 0x0F);
    out_byte(0x3D5, (uint8_t) (pos & 0xFF));
    out_byte(0x3D4, 0x0E);
    out_byte(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/**
 * Output a character to the screen. Interpret '\n', '\r', and 't'.
 * The cursor position is updated but the visual is not.
*/
static void put_char(char c) 
{
    switch (c) {
    case '\n':
        cursor_y++;
        cursor_x = 0;
        break;
    case '\r':
        cursor_x = 0;
        break;
    case '\t':
        cursor_x += 4;
        cursor_x -= cursor_x % 4;
        break;
    default:
        set_char(cursor_x, cursor_y, c);
        cursor_x++;
    }

    // Wrap to a new line if past end of screen
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= SCREEN_HEIGHT) {
        vga_scrollback(1);
    }
}

/**
 * Write '\0' to each character point on the screen. Reset the cursor to 0, 0
*/
void vga_clear_screen() 
{
    for (int y=0; y<SCREEN_HEIGHT; y++) {
        for (int x=0; x<SCREEN_WIDTH; x++) {
            set_char(x, y, '\0');
        }
    }
    
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

/**
 * Copy [lines] lines up and clear the bottom [lines] lines.
*/
void vga_scrollback(int lines) 
{
    for (int y = lines; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            set_char(x, y-lines, get_char(x, y));
        }
    }

    for (int y = SCREEN_HEIGHT-lines; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            set_char(x, y, '\0');
        }
    }

    cursor_y -= lines;
    update_cursor();
}

/**
 * Output a character to the screen and advance the cursor.
*/
void vga_putc(char c) 
{
    put_char(c);
    update_cursor();
}


