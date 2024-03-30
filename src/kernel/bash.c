#include "bash.h"

#include <stdio.h>
#include "keyboard.h"
#include <string.h>
#include <stdlib.h>
#include <arch/i686/io.h>

static char command[80];
static uint8_t command_length;

static uint32_t to_int(char **string) {
    uint32_t value = 0;
    char c = **string;
    while (true) {
        uint32_t d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 0xA;
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 0xA;
        else break;
        value = value * 0x10 + d;
        (*string)++;
        if (*string >= command + 80) break;
        c = **string;
    }
    return value;
}

static void cmd_echo() {
    for (int i=5; i<80; i++) {
        if (command[0] == '\0') break;
        printf("%c", command[i]);
    }
    printf("\n");
}

static void cmd_out(int size) {
    char *next = command + 5;
    uint32_t port = to_int(&next);
    next++;
    uint32_t value = to_int(&next);
    printf("[%#x] <- %#x\n", port, value);
    switch (size)
    {
    case 8:     out_byte(port, value);
                break;
    case 16:    out_word(port, value);
                break;
    case 32:    out_double(port, value);
    }
}

static void cmd_in(int size) {
    char *next = command + 4;
    uint32_t port = to_int(&next);
    uint8_t value;
    switch (size)
    {
    case 8:     value = in_byte(port);
                break;
    case 16:    value = in_word(port);
                break;
    case 32:    value = in_double(port);
    }
    printf("[%#x] -> %#x\n", port, value);
}

static void cmd_unkown() {
    printf("Invalid Command\n");
}

static void process_command() {
    if (command_length == 0) {
        printf("> ");
        return;
    }

    if (memcmp(command, "echo ", 5) == 0) cmd_echo();
    else if (memcmp(command, "outb ", 5) == 0) cmd_out(8);
    else if (memcmp(command, "outw ", 5) == 0) cmd_out(16);
    else if (memcmp(command, "outd ", 5) == 0) cmd_out(32);
    else if (memcmp(command, "inb ", 4) == 0) cmd_in(8);
    else if (memcmp(command, "inw ", 4) == 0) cmd_in(8);
    else if (memcmp(command, "ind ", 4) == 0) cmd_in(8);
    else cmd_unkown();

    for (; command_length > 0; command_length--)
        command[command_length] = ' ';
    command[command_length] = ' ';
    printf("> ");
}

static void keypress_handler(void *args) {
    KeypressEvent *event = args;
    if (event->ascii != '\0' && !event->released) {
        if (event->ascii == '\b') {
            if (command_length > 0) {
                command_length--;
                printf("%c", '\b');
            }
        } else {
            printf("%c", event->ascii);
            if (event->ascii == '\n') {
                process_command();
            } else {
                command[command_length++] = event->ascii;
            }
        }
    }
    free(event);
}

void bash_initialize() {
    printf("\n");
    printf("> ");

    register_handler(keypress_handler);
}