#include "stdio.h"

#include <stdint.h>
#include <stdbool.h>

const unsigned SCREEN_WIDTH = 80;
const unsigned SCREEN_HEIGHT = 25;
const uint8_t DEFAULT_COLOR = 0x7;

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

void puts(const char* str) {
    while (*str) {
        putc(*str);
        str++;
    }
}

char* hexchars = "0123456789ABCDEF";
char printbuf[256];

void puthex(uint64_t value) {

    int count = 0;
    if (value == 0) {
        printbuf[count] = hexchars[0];
        count++;
    }

    while (value > 0) {
        uint8_t digit = value % 16;
        value = value / 16;
        printbuf[count] = hexchars[digit];
        count++;
    }

    count--;

    while (count >= 0) {
        putc(printbuf[count]);
        count--;
    }
}

void clear_screen() {
    for (int i=0; i<SCREEN_WIDTH; i++) {
        for (int j=0; j<SCREEN_HEIGHT; j++) {
            putchr(i, j, ' ');
            putcolor(i, j, DEFAULT_COLOR);
        }
    }
    g_ScreenX = 0;
    g_ScreenY = 0;
}

enum PrintfState {
    STATE_NORMAL,
    STATE_FLAGS,
    STATE_WIDTH,
    STATE_PRECISION,
    STATE_LENGTH,
    STATE_SPECIFIER
};

enum PrintfLength {
    LENGTH_NORMAL,
    LENGTH_SHORT,
    LENGTH_SHORT_SHORT,
    LENGTH_LONG,
    LENGTH_LONG_LONG
};

enum PrinfFlags {
    FLAG_DEFAULT = 0,
    FLAG_LEFT_JUSTIFY = 1,
    FLAG_SIGN = 2,
    FLAG_SPACE = 4,
    FLAG_ALT_FORM = 8,
    FLAG_ZERO_PAD = 16,
    FLAG_GROUP = 32
};

void print_num(long long num, int radix, int width, int precision, bool sign, int flags) {
    char buffer[64];
    int pos = 0;

    bool negative = num < 0;
    bool leftJustify = flags & FLAG_LEFT_JUSTIFY;
    bool padZeros = flags & FLAG_ZERO_PAD;
    bool hasSign = sign && (negative || (flags & FLAG_SIGN));
    bool altForm = (flags & FLAG_ALT_FORM);
    char signChar = (sign && negative) ? '-' : '+';

    unsigned long long absNum;
    if (sign && negative) {
        absNum = (unsigned long long)(-num);
    } else {
        absNum = (unsigned long long)(num);
    }


    if (!hasSign && (flags & FLAG_SPACE)) {
        hasSign = true;
        signChar = ' ';
    }

    // Convert number to string
    do {
        unsigned long long rem = absNum % radix;
        absNum /= radix;
        buffer[pos++] = hexchars[rem];
    } while (absNum > 0);


    // Determine amount of padding necessary for width
    int padding = width - precision;
    if (pos > precision) padding = width - pos;
    char padChar = padZeros ? '0' : ' ';

    if (!leftJustify && padding > 0) {
        if (padZeros) {
            if (hasSign) {
                putc(signChar);    // If sign is at front then write the sign
                padding--;
            }

            if (altForm) {
                if (radix == 8 && num == 0) {
                    putc('0');
                    padding--;
                } else if (radix == 16) {
                    putc('0');
                    putc('x');
                    padding -= 2;
                }
            }

            while (padding > 0) {
                putc(padChar);
                padding--;
            }
        } else {
            if (hasSign) padding--;
            if (altForm) padding -= 2;

            while (padding > 0) {
                putc(padChar);
                padding--;
            }

            if (hasSign) putc(signChar);
            if (altForm) {
                if (radix == 8 && num == 0) {
                    putc('0');
                } else if (radix == 16) {
                    putc('0');
                    putc('x');
                }
            }
        }
    } else  {
        if (hasSign) {
            putc(signChar);
            padding--;
        }

        if (flags & FLAG_ALT_FORM) {
            if (radix == 8 && num == 0) {
                putc('0');
                padding--;
            } else if (radix == 16) {
                putc('0');
                putc('x');
                padding -= 2;
            }
        }
    }
    

    // Output extra zeros for precision
    while (precision > pos) {
        putc('0');
        precision--;
    }

    // Output the number
    if (precision != 0 || num != 0) {
        while (--pos >= 0)
            putc(buffer[pos]);
    }
    

    // Output any ending padding
    while (padding > 0) {
        putc(padChar);
        padding--;
    }
}

int parseint(const char** str) {
    int result = 0;
    char current = **str;
    while (0x30 <= current && current <=0x39) {
        result *= 10;
        result += current - 0x30;
        (*str)++;
        current = **str;
    }

    return result;
}

void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    enum PrintfState state = STATE_NORMAL;
    enum PrintfLength length = LENGTH_NORMAL;
    int flags = FLAG_DEFAULT;

    int radix = 10;
    int width = 0;
    int precision = -1;
    bool sign = false;
    bool number = false;
    bool advanceChar;

    while (*fmt) {
        advanceChar = true;
        switch (state) {
            case STATE_NORMAL:
                switch (*fmt) {
                    case '%': 
                        state = STATE_FLAGS;
                        break;
                    default:
                        putc(*fmt);
                        break;
                }
                break;
            case STATE_FLAGS:
                switch (*fmt) {
                    case '-':   flags |= FLAG_LEFT_JUSTIFY;
                                break;
                    case '+':   flags |= FLAG_SIGN;
                                break;
                    case ' ':   flags |= FLAG_SPACE;
                                break;
                    case '#':   flags |= FLAG_ALT_FORM;
                                break;
                    case '0':   flags |= FLAG_ZERO_PAD;
                                break;
                    case '\'':  flags |= FLAG_GROUP;
                                break;
                    default:    advanceChar = false;       
                                state = STATE_WIDTH;
                }
                break;
            case STATE_WIDTH:
                if (*fmt == '*') {
                    const char* arg = va_arg(args, const char*);
                    width = parseint(&arg);
                    fmt++;
                } else {
                    width = parseint(&fmt);
                }

                if (*fmt == '.') {
                    state = STATE_PRECISION;
                } else {
                    state = STATE_LENGTH;
                    advanceChar = false;
                }
                break;
            case STATE_PRECISION:
                if (*fmt == '*') {
                    const char* arg = va_arg(args, const char*);
                    precision = parseint(&arg);
                    fmt++;
                } else {
                    precision = parseint(&fmt);
                }
                state = STATE_LENGTH;
                advanceChar = false;
                break;
            case STATE_LENGTH:
                switch (*fmt) {
                    case 'h':
                        if (length == LENGTH_SHORT_SHORT) state = STATE_SPECIFIER;
                        length = length == LENGTH_NORMAL ? LENGTH_SHORT : LENGTH_SHORT_SHORT;
                        break;
                    case 'l':
                        if (length == LENGTH_SHORT_SHORT) state = STATE_SPECIFIER;
                        length = length == LENGTH_NORMAL ? LENGTH_LONG : LENGTH_LONG_LONG;
                        break;
                    default:
                        advanceChar = false;
                        state = STATE_SPECIFIER;
                }
                break;
            case STATE_SPECIFIER:
                switch (*fmt) {
                    case 'c':
                        putc((char)va_arg(args, int));
                        break;
                    case 's':
                        puts(va_arg(args, const char*));
                        break;
                    case '%':
                        putc('%');
                        break;
                    case 'd':
                    case 'i':
                        radix = 10;
                        sign = true;
                        number = true;
                        break;
                    case 'u':
                        radix = 10;
                        sign = false;
                        number = true;
                        break;
                    case 'X':
                    case 'x':
                    case 'p':
                        radix = 16;
                        sign = false;
                        number = true;
                        break;
                    case 'o':
                        radix = 8;
                        sign = false;
                        number = true;
                        break;
                
                    default:
                        break;
                }

                if (number) {
                    switch (length) {
                        case LENGTH_SHORT_SHORT:
                        case LENGTH_SHORT:
                        case LENGTH_NORMAL:
                            print_num(va_arg(args, int), radix, width, precision, sign, flags);
                            break;
                        case LENGTH_LONG:
                            print_num(va_arg(args, long), radix, width, precision, sign, flags);
                            break;
                        case LENGTH_LONG_LONG:
                            print_num(va_arg(args, long long), radix, width, precision, sign, flags);
                            break;
                    }
                }

                state = STATE_NORMAL;
                length = LENGTH_NORMAL;
                radix = 10;
                width = 0;
                precision = -1;
                sign = false;
                number = false;
                break;
        }

        if (advanceChar) fmt++;
    }

    va_end(args); 
}