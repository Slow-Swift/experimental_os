#include "format_print.h"

enum PrintFormatState {
    STATE_NORMAL,
    STATE_FLAGS,
    STATE_WIDTH,
    STATE_PRECISION,
    STATE_LENGTH,
    STATE_SPECIFIER
};

enum PrintFormatLength {
    LENGTH_NORMAL,
    LENGTH_SHORT,
    LENGTH_SHORT_SHORT,
    LENGTH_LONG,
    LENGTH_LONG_LONG
};

enum PrintFormatFlags {
    FLAG_DEFAULT = 0,
    FLAG_LEFT_JUSTIFY = 1,
    FLAG_SIGN = 2,
    FLAG_SPACE = 4,
    FLAG_ALT_FORM = 8,
    FLAG_ZERO_PAD = 16,
    FLAG_GROUP = 32
};

char* hexchars = "0123456789ABCDEF";

/**
 * Print out [num] in base [radix] signed according to [sign] and accounting
 * for width, precision, and flags according to the fprintf specfication.
 * [printc] with stream argument [stream] is used to print the characters.
 * 
 * Parameters:
 *   num: The number to print
 *   radix: The base to print the number in
 *   width: The minimum number of character to print
 *   precision: The minimum number of digits to print
 *   sign: Whether or not the number is signed
 *   flags: Flags used to change how printing is done
 *   stream: The stream to pass to [printc]
 *   printc: The function to use for printing characters
 * 
 * Returns:
 *   The number of characters printed
*/
int print_num(
    long long num, int radix, int width, int precision, bool sign, int flags,
    FILE * restrict stream, char_printer printc
) {
    char buffer[64];
    int pos = 0;
    int char_count = 0;

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
    if (pos > precision) 
        padding = width - pos;
    char padChar = padZeros ? '0' : ' ';

    if (!leftJustify && padding > 0) {
        if (padZeros) {
            if (hasSign) {
                printc(signChar, stream);
                char_count++;
                padding--;
            }

            if (altForm) {
                if (radix == 8 && num == 0) {
                    printc('0', stream);
                    char_count++;
                    padding--;
                } else if (radix == 16) {
                    printc('0', stream);
                    printc('x', stream);
                    char_count += 2;
                    padding -= 2;
                }
            }

            while (padding > 0) {
                printc(padChar, stream);
                char_count++;
                padding--;
            }
        } else {
            if (hasSign) padding--;
            if (altForm) padding -= 2;

            while (padding > 0) {
                printc(padChar, stream);
                char_count++;
                padding--;
            }

            if (hasSign) {
                printc(signChar, stream);
                char_count++;
            }

            if (altForm) {
                if (radix == 8 && num == 0) {
                    printc('0', stream);
                    char_count++;
                } else if (radix == 16) {
                    printc('0', stream);
                    printc('x', stream);
                    char_count += 2;
                }
            }
        }
    } else  {
        if (hasSign) {
            printc(signChar, stream);
            char_count++;
            padding--;
        }

        if (flags & FLAG_ALT_FORM) {
            if (radix == 8 && num == 0) {
                printc('0', stream);
                char_count++;
                padding--;
            } else if (radix == 16) {
                printc('0', stream);
                printc('x', stream);
                char_count += 2;
                padding -= 2;
            }
        }
    }
    

    // Output extra zeros for precision
    while (precision > pos) {
        printc('0', stream);
        char_count++;
        precision--;
    }

    // Output the number
    if (precision != 0 || num != 0) {
        while (--pos >= 0) {
            printc(buffer[pos], stream);
            char_count++;
        }
    }
    

    // Output any ending padding
    while (padding > 0) {
        printc(padChar, stream);
        char_count++;
        padding--;
    }

    return char_count;
}

/**
 * Read an base 10 positive integer from the string pointed to by the pointer 
 * pointed to by [str]. The pointer pointed to by [str] is updated to point to 
 * the next character after the read number.
*/
static int parseint(const char** str) {
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

int format_print(
    const char *fmt, FILE * restrict stream, char_printer printc, va_list arg
) {
    enum PrintFormatState state = STATE_NORMAL;
    enum PrintFormatLength length = LENGTH_NORMAL;
    int flags = FLAG_DEFAULT;

    int char_count = 0;
    int radix = 10;
    int width = 0;
    int precision = -1;
    bool has_sign = false;
    bool is_number = false;
    bool advance_char;

    while (*fmt) {
        advance_char = true;
        switch (state) {
        case STATE_NORMAL:
            switch (*fmt) {
            case '%': 
                state = STATE_FLAGS;
                break;
            default:
                printc(*fmt, stream);
                char_count++;
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
            default:    advance_char = false;       
                        state = STATE_WIDTH;
            }
            break;
        case STATE_WIDTH:
            if (*fmt == '*') {
                width = va_arg(arg, int);
                fmt++;
            } else {
                width = parseint(&fmt);
            }

            if (*fmt == '.') {
                state = STATE_PRECISION;
            } else {
                state = STATE_LENGTH;
                advance_char = false;
            }
            break;
        case STATE_PRECISION:
            if (*fmt == '*') {
                precision = va_arg(arg, int);
                fmt++;
            } else {
                precision = parseint(&fmt);
            }
            state = STATE_LENGTH;
            advance_char = false;
            break;
        case STATE_LENGTH:
            switch (*fmt) {
            case 'h':
                if (length == LENGTH_SHORT_SHORT) state = STATE_SPECIFIER;
                length = length == LENGTH_NORMAL 
                         ? LENGTH_SHORT 
                         : LENGTH_SHORT_SHORT;
                break;
            case 'l':
                if (length == LENGTH_SHORT_SHORT) state = STATE_SPECIFIER;
                length = length == LENGTH_NORMAL 
                         ? LENGTH_LONG 
                         : LENGTH_LONG_LONG;
                break;
            default:
                advance_char = false;
                state = STATE_SPECIFIER;
            }
            break;
        case STATE_SPECIFIER:
            switch (*fmt) {
            case 'c':
                printc(va_arg(arg, int), stream);
                char_count++;
                break;
            case 's':
                const char *s = va_arg(arg, const char*);
                while (*s) {
                    printc(*s, stream);
                    *s++;
                    char_count++;
                }
                break;
            case '%':
                printc('%', stream);
                char_count++;
                break;
            case 'd':
            case 'i':
                radix = 10;
                has_sign = true;
                is_number = true;
                break;
            case 'u':
                radix = 10;
                has_sign = false;
                is_number = true;
                break;
            case 'X':
            case 'x':
            case 'p':
                radix = 16;
                has_sign = false;
                is_number = true;
                break;
            case 'o':
                radix = 8;
                has_sign = false;
                is_number = true;
                break;
            default:
                break;
            }

            if (is_number) {
                switch (length) {
                case LENGTH_SHORT_SHORT:
                case LENGTH_SHORT:
                case LENGTH_NORMAL:
                    char_count += print_num(
                        va_arg(arg, int), radix, width, precision, 
                        has_sign, flags, stream, printc
                    );
                    break;
                case LENGTH_LONG:
                    char_count += print_num(
                        va_arg(arg, long), radix, width, precision, 
                        has_sign, flags, stream, printc
                    );
                    break;
                case LENGTH_LONG_LONG:
                    char_count += print_num(
                        va_arg(arg, long long), radix, width, precision, 
                        has_sign, flags, stream, printc
                    );
                    break;
                }
            }

            state = STATE_NORMAL;
            length = LENGTH_NORMAL;
            radix = 10;
            width = 0;
            precision = -1;
            has_sign = false;
            is_number = false;
            break;
        }

        if (advance_char) fmt++;
    }

    return char_count;
}