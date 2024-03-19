#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    NONE,
    UNKOWN,
    AT_KBD,
    PS_2_MOUSE,
    SCROLL_MOUSE,
    BUTTON_5_MOUSE,
    MF2_KBD,
    SHORT_KBD,
    HOST_KBD_122_KEY,
    KBD_122_KEY,
    JAPANESE_G_KBD,
    JAPANESE_P_KBD,
    JAPANESE_A_KBD,
    NCD_SUN_KBD
} PS2_Device_Type;

typedef struct {
    int port;
    bool present;
    PS2_Device_Type type;
    int (*send_byte)(uint8_t);
    void (*on_byte_recieved)(uint8_t);
} PS2_Device;

void ps2_initialize();
bool ps2_configured_ok();
PS2_Device* ps2_get_port_1_device();
PS2_Device* ps2_get_port_2_device();