#pragma once

#include <stdbool.h>
#include "events.h"

typedef enum {
    KEYCODE_ESC     = 0x00,
    KEYCODE_F1      = 0x01,
    KEYCODE_F2      = 0x02,
    KEYCODE_F3      = 0x03,
    KEYCODE_F4      = 0x04,
    KEYCODE_F5      = 0x05,
    KEYCODE_F6      = 0x06,
    KEYCODE_F7      = 0x07,
    KEYCODE_F8      = 0x08,
    KEYCODE_F9     = 0x09,
    KEYCODE_F10     = 0x0A,
    KEYCODE_F11     = 0x0B,
    KEYCODE_F12     = 0x0C,

    KEYCODE_BACKTICK    = 0x20,
    KEYCODE_1       = 0x21,
    KEYCODE_2       = 0x22,
    KEYCODE_3       = 0x23,
    KEYCODE_4       = 0x24,
    KEYCODE_5       = 0x25,
    KEYCODE_6       = 0x26,
    KEYCODE_7       = 0x27,
    KEYCODE_8       = 0x28,
    KEYCODE_9       = 0x29,
    KEYCODE_0       = 0x2A,
    KEYCODE_MINUS   = 0x2B,
    KEYCODE_EQUAL   = 0x2C,
    KEYCODE_BACKSPACE = 0x2D,

    KEYCODE_TAB     = 0x40,
    KEYCODE_Q       = 0x41,
    KEYCODE_W       = 0x42,
    KEYCODE_E       = 0x43,
    KEYCODE_R       = 0x44,
    KEYCODE_T       = 0x45,
    KEYCODE_Y       = 0x46,
    KEYCODE_U       = 0x47,
    KEYCODE_I       = 0x48,
    KEYCODE_O       = 0x49,
    KEYCODE_P       = 0x4A,
    KEYCODE_LBRACKET     = 0x4B,
    KEYCODE_RBRACKET     = 0x4C,
    KEYCODE_BSLASH       = 0x4D,

    KEYCODE_CAPS    = 0x60,
    KEYCODE_A       = 0x61,
    KEYCODE_S       = 0x62,
    KEYCODE_D       = 0x63,
    KEYCODE_F       = 0x64,
    KEYCODE_G       = 0x65,
    KEYCODE_H       = 0x66,
    KEYCODE_J       = 0x67,
    KEYCODE_K       = 0x68,
    KEYCODE_L       = 0x69,
    KEYCODE_SEMICOLON   = 0x6A,
    KEYCODE_QUOT        = 0x6B,
    KEYCODE_ENTER   = 0x6C,

    KEYCODE_LSHIFT  = 0x80,
    KEYCODE_Z       = 0x81,
    KEYCODE_X       = 0x82,
    KEYCODE_C       = 0x83,
    KEYCODE_V       = 0x84,
    KEYCODE_B       = 0x85,
    KEYCODE_N       = 0x86,
    KEYCODE_M       = 0x87,
    KEYCODE_COMMA   = 0x88,
    KEYCODE_PERIOD  = 0x89,
    KEYCODE_SLASH   = 0x8A,
    KEYCODE_RSHIFT  = 0x8B,

    KEYCODE_LCTRL   = 0xA0,
    KEYCODE_LGUI    = 0xA1,
    KEYCODE_LALT    = 0xA2,
    KEYCODE_SPACE   = 0xA3,
    KEYCODE_RALT    = 0xA4,
    KEYCODE_RGUI    = 0xA5,
    KEYCODE_RCTRL   = 0xA6,
    KEYCODE_FN      = 0xA7,

    KEYCODE_PRINTSCR    = 0x0D,
    KEYCODE_SCLLK       = 0x0E,

    KEYCODE_INSERT      = 0x2E,
    KEYCODE_HOME        = 0x2F,
    KEYCODE_PGUP        = 0x30,
    KEYCODE_DEL         = 0x4E,
    KEYCODE_END         = 0x4F,
    KEYCODE_PGDWN       = 0x50,

    KEYCODE_UP_ARROW    = 0x8F,
    KEYCODE_LEFT_ARROW  = 0xAE,
    KEYCODE_DOWN_ARROW  = 0xAF,
    KEYCODE_RIGHT_ARROW = 0xB0,

    KEYCODE_NUMLOCK     = 0x32,
    KEYCODE_PAD_SLASH   = 0x33,
    KEYCODE_PAD_STAR    = 0x34,
    KEYCODE_PAD_7       = 0x51,
    KEYCODE_PAD_8       = 0x52,
    KEYCODE_PAD_9       = 0x53,
    KEYCODE_PAD_MINUS   = 0x54,
    KEYCODE_PAD_4       = 0x71,
    KEYCODE_PAD_5       = 0x72,
    KEYCODE_PAD_6       = 0x73,
    KEYCODE_PAD_PLUS    = 0x74,
    KEYCODE_PAD_1       = 0x91,
    KEYCODE_PAD_2       = 0x92,
    KEYCODE_PAD_3       = 0x93,
    KEYCODE_PAD_0       = 0xB2,
    KEYCODE_PAD_DOT     = 0xB3,
    KEYCODE_PAD_ENTER   = 0xB4,

    KEYCODE_MM_WWW          = 0xC0,
    KEYCODE_MM_WWW_FAV      = 0xC1,
    KEYCODE_MM_WWW_REFRESH  = 0xC2,
    KEYCODE_MM_WWW_STOP     = 0xC3,
    KEYCODE_MM_WWW_FORWARD  = 0xC4,
    KEYCODE_MM_WWW_BACK     = 0xC5,
    KEYCODE_MM_WWW_HOME     = 0xC6,
    KEYCODE_MM_PREV_TRACK   = 0xC7,
    KEYCODE_MM_PAUSE        = 0xC8,
    KEYCODE_MM_NEXT_TRACK   = 0xC9,
    KEYCODE_MM_STOP         = 0xCA,
    KEYCODE_MM_MEDIA_SEL    = 0xCB,
    KEYCODE_MM_VDOWN        = 0xCC,
    KEYCODE_MM_MUTE         = 0xCD,
    KEYCODE_MM_VUP          = 0xCE,
    KEYCODE_MM_CALCULATOR   = 0xCF,
    KEYCODE_MM_COMPUTER     = 0xD0,
    KEYCODE_MM_EMAIL        = 0xD1,
    KEYCODE_APPS            = 0xD2,
    KEYCODE_ACPI_POWER      = 0xD3,
    KEYCODE_ACPI_SLEEP      = 0xD4,
    KEYCODE_ACPI_WAKE       = 0xD5,
    KEYCODE_PAUSE           = 0xD6,

    KEYCODE_MOUSE_LEFT      = 0xE0,
    KEYCODE_MOUSE_MIDDLE    = 0xE1,
    KEYCODE_MOUSE_RIGHT     = 0xE2,
    KEYCODE_MOUSE_UP        = 0xE3,
    KEYCODE_MOUSE_DOWN      = 0xE4,
} Keycode;

typedef struct {
    char ascii;
    Keycode code;
    bool released;
    struct {
        bool shift;
        bool alt;
        bool control;
        bool function;
        bool gui;
    } modifiers;

    struct {
        bool caps_lock;
        bool num_lock;
        bool scroll_lock;
    } toggles;
} KeypressEvent;

void kbd_initialize();

void register_handler(Event_Handler handler);