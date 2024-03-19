#include "keyboard.h"

#include <arch/i686/ps2.h>
#include "events.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_COMMANDS 10

static enum {
    PORT_DATA   = 0x60,
    PORT_CMD    = 0x64
} KBD_PORTS;

typedef enum {
    CMD_SCAN_CODE_SET   = 0xF0,
    CMD_ENABLE_SCAN     = 0xF4,
    CMD_DISABLE_SCAN    = 0xF5
} KbdCmdCode;

static enum {
    RESPONSE_ACK    = 0xFA,
    RESPONSE_RESEND = 0xFE
} KBD_RESPONSE;

typedef enum {
    INIT_START,
    DISABLE_SCAN,
    SET_SCAN_SET_3,
    GET_SCAN_SET,
    ENABLE_SCAN,
    INITIALIZED
} Initialization_State;

typedef struct {
    KbdCmdCode code;
    uint8_t subcommand;
    bool has_subcommand;
    bool has_response;
    void (*on_complete)(bool, uint8_t);
    bool sub_sent;
    bool acknowledged;
    int resend_count;
} KbdCommand;

static KbdCommand cmd_queue[MAX_COMMANDS];
static int queue_min = 0, queue_count = 0;

static int scan_code_set;

static PS2_Device* device;
static bool initialized = false;
static Initialization_State init_state = INIT_START;

static uint8_t pressed_keys[64];
static uint8_t scancode_bytes[8];
static int scancode_byte_count = 0;
static Event_Handler keypress_handler = NULL;

static struct {
    bool shift;
    bool alt;
    bool control;
    bool function;
    bool gui;
} modifiers;

static struct {
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
} toggles;

static uint8_t scancode_set_2[] = {
    [0x01] = KEYCODE_F9,
    [0x03] = KEYCODE_F5,
    [0x04] = KEYCODE_F3,
    [0x05] = KEYCODE_F1,
    [0x06] = KEYCODE_F2,
    [0x07] = KEYCODE_F12,
    [0x09] = KEYCODE_F10,
    [0x0A] = KEYCODE_F8,
    [0x0B] = KEYCODE_F6,
    [0x0C] = KEYCODE_F4,

    [0x0D] = KEYCODE_TAB,
    [0x0E] = KEYCODE_BACKTICK,

    [0x11] = KEYCODE_LALT,
    [0x12] = KEYCODE_LSHIFT,
    [0x14] = KEYCODE_LCTRL,	    [0x15] = KEYCODE_Q,	    [0x16] = KEYCODE_1,		
    [0x1A] = KEYCODE_Z,  	    [0x1B] = KEYCODE_S,
    [0x1C] = KEYCODE_A,  	    [0x1D] = KEYCODE_W,	    [0x1E] = KEYCODE_2,		
    [0x21] = KEYCODE_C,  	    [0x22] = KEYCODE_X,	    [0x23] = KEYCODE_D,
    [0x24] = KEYCODE_E,  	    [0x25] = KEYCODE_4,	    [0x26] = KEYCODE_3,		
    [0x29] = KEYCODE_SPACE,	    [0x2A] = KEYCODE_V,	    [0x2B] = KEYCODE_F,
    [0x2C] = KEYCODE_T,  	    [0x2D] = KEYCODE_R,	    [0x2E] = KEYCODE_5,		
    [0x31] = KEYCODE_N,  	    [0x32] = KEYCODE_B,	    [0x33] = KEYCODE_H,
    [0x34] = KEYCODE_G,	        [0x35] = KEYCODE_Y,	    [0x36] = KEYCODE_6,		
    [0x3A] = KEYCODE_M,  	    [0x3B] = KEYCODE_J,
    [0x3C] = KEYCODE_U,  	    [0x3D] = KEYCODE_7,	    [0x3E] = KEYCODE_8,		
    [0x41] = KEYCODE_COMMA,	    [0x42] = KEYCODE_K,	    [0x43] = KEYCODE_I,
    [0x44] = KEYCODE_O,	        [0x45] = KEYCODE_0,	    [0x46] = KEYCODE_9,		
    [0x49] = KEYCODE_PERIOD,	[0x4A] = KEYCODE_SLASH,	[0x4B] = KEYCODE_L,
    [0x4C] = KEYCODE_SEMICOLON,	[0x4D] = KEYCODE_P,	    [0x4E] = KEYCODE_MINUS,		
    [0x52] = KEYCODE_QUOT,		
    [0x54] = KEYCODE_LBRACKET,	[0x55] = KEYCODE_EQUAL,				
    [0x58] = KEYCODE_CAPS,	    [0x59] = KEYCODE_RSHIFT,    
    [0x5A] = KEYCODE_ENTER,	    [0x5B] = KEYCODE_RBRACKET,
    [0x5D] = KEYCODE_BSLASH,	
    [0x66] = KEYCODE_BACKSPACE,		
    [0x69] = KEYCODE_PAD_1,		[0x6B] = KEYCODE_PAD_4,
    [0x6C] = KEYCODE_PAD_7,
    [0x70] = KEYCODE_PAD_0,  	[0x71] = KEYCODE_PAD_DOT,	
    [0x72] = KEYCODE_PAD_2,     [0x73] = KEYCODE_PAD_5,
    [0x74] = KEYCODE_PAD_7, 	[0x75] = KEYCODE_PAD_8, 	
    [0x76] = KEYCODE_ESC,	    [0x77] = KEYCODE_NUMLOCK,
    [0x78] = KEYCODE_F11,   	[0x79] = KEYCODE_PAD_PLUS,	
    [0x7A] = KEYCODE_PAD_3, 	[0x7B] = KEYCODE_PAD_MINUS,
    [0x7C] = KEYCODE_PAD_STAR,	[0x7D] = KEYCODE_PAD_9, 	
    [0x7E] = KEYCODE_SCLLK,
    [0x83] = KEYCODE_F7
};

static uint8_t e_scancode_set_2[] = {
    [0x10] = KEYCODE_MM_WWW,
    [0x11] = KEYCODE_RALT,
    [0x14] = KEYCODE_RCTRL,
    [0x15] = KEYCODE_MM_PREV_TRACK,
    [0x18] = KEYCODE_MM_WWW_FAV,
    [0x1F] = KEYCODE_LGUI,
    [0x20] = KEYCODE_MM_WWW_REFRESH,
    [0x21] = KEYCODE_MM_VDOWN,
    [0x23] = KEYCODE_MM_MUTE,
    [0x27] = KEYCODE_RGUI,
    [0x28] = KEYCODE_MM_WWW_STOP,
    [0x2B] = KEYCODE_MM_CALCULATOR,
    [0x2F] = KEYCODE_APPS,
    [0x30] = KEYCODE_MM_WWW_FORWARD,
    [0x32] = KEYCODE_MM_VUP,
    [0x34] = KEYCODE_MM_PAUSE,
    [0x37] = KEYCODE_ACPI_POWER,
    [0x38] = KEYCODE_MM_WWW_BACK,
    [0x3A] = KEYCODE_MM_WWW_HOME,
    [0x3B] = KEYCODE_MM_STOP,
    [0x3F] = KEYCODE_ACPI_SLEEP,
    [0x40] = KEYCODE_MM_COMPUTER,
    [0x48] = KEYCODE_MM_EMAIL,
    [0x4A] = KEYCODE_PAD_SLASH,
    [0x4D] = KEYCODE_MM_NEXT_TRACK,
    [0x50] = KEYCODE_MM_MEDIA_SEL,
    [0x54] = KEYCODE_PAD_ENTER,
    [0x5E] = KEYCODE_ACPI_WAKE,
    [0x69] = KEYCODE_END,
    [0x6B] = KEYCODE_MOUSE_LEFT,
    [0x6C] = KEYCODE_HOME,
    [0x70] = KEYCODE_INSERT,
    [0x71] = KEYCODE_DEL,
    [0x72] = KEYCODE_MOUSE_DOWN,
    [0x73] = KEYCODE_MOUSE_RIGHT,
    [0x74] = KEYCODE_MOUSE_UP,
    [0x7A] = KEYCODE_PGDWN,
    [0x7D] = KEYCODE_PGUP,
};

static uint8_t scancode_set_2_pause[] = 
    { 0xE1, 0x14, 0x77, 0xE1, 0xF0, 0x14, 0xF0, 0x77 };

static char lower_ascii[] = {
    [KEYCODE_BACKTICK]  = '`',
    [KEYCODE_1]         = '1',
    [KEYCODE_2]         = '2',
    [KEYCODE_3]         = '3',
    [KEYCODE_4]         = '4',
    [KEYCODE_5]         = '5',
    [KEYCODE_6]         = '6',
    [KEYCODE_7]         = '7',
    [KEYCODE_8]         = '8',
    [KEYCODE_9]         = '9',
    [KEYCODE_0]         = '0',
    [KEYCODE_MINUS]     = '-',
    [KEYCODE_EQUAL]     = '=',
    [KEYCODE_BACKSPACE] = '\b',

    [KEYCODE_TAB]       = '\t',
    [KEYCODE_Q]         = 'q',
    [KEYCODE_W]         = 'w',
    [KEYCODE_E]         = 'e',
    [KEYCODE_R]         = 'r',
    [KEYCODE_T]         = 't',
    [KEYCODE_Y]         = 'y',
    [KEYCODE_U]         = 'u',
    [KEYCODE_I]         = 'i',
    [KEYCODE_O]         = 'o',
    [KEYCODE_P]         = 'p',
    [KEYCODE_LBRACKET]  = '[',
    [KEYCODE_RBRACKET]  = ']',
    [KEYCODE_BSLASH]    = '\\',

    [KEYCODE_A]         = 'a',
    [KEYCODE_S]         = 's',
    [KEYCODE_D]         = 'd',
    [KEYCODE_F]         = 'f',
    [KEYCODE_G]         = 'g',
    [KEYCODE_H]         = 'h',
    [KEYCODE_J]         = 'j',
    [KEYCODE_K]         = 'k',
    [KEYCODE_L]         = 'l',
    [KEYCODE_SEMICOLON] = ';',
    [KEYCODE_QUOT]      = '\'',
    [KEYCODE_ENTER]     = '\n',

    [KEYCODE_Z]         = 'z',
    [KEYCODE_X]         = 'x',
    [KEYCODE_C]         = 'c',
    [KEYCODE_V]         = 'v',
    [KEYCODE_B]         = 'b',
    [KEYCODE_N]         = 'n',
    [KEYCODE_M]         = 'm',
    [KEYCODE_COMMA]     = ',',
    [KEYCODE_PERIOD]    = '.',
    [KEYCODE_SLASH]     = '/',
    [KEYCODE_SPACE]     = ' ',

    [KEYCODE_PAD_SLASH] = '/',
    [KEYCODE_PAD_STAR]  = '*',
    [KEYCODE_PAD_7]     = '7',
    [KEYCODE_PAD_8]     = '8',
    [KEYCODE_PAD_9]     = '9',
    [KEYCODE_PAD_MINUS] = '-',
    [KEYCODE_PAD_4]     = '4',
    [KEYCODE_PAD_5]     = '5',
    [KEYCODE_PAD_6]     = '6',
    [KEYCODE_PAD_PLUS]  = '+',
    [KEYCODE_PAD_1]     = '1',
    [KEYCODE_PAD_2]     = '2',
    [KEYCODE_PAD_3]     = '3',
    [KEYCODE_PAD_0]     = '0',
    [KEYCODE_PAD_DOT]   = '.'
};

static char upper_ascii[] = {
    [KEYCODE_BACKTICK]  = '~',
    [KEYCODE_1]         = '!',
    [KEYCODE_2]         = '@',
    [KEYCODE_3]         = '#',
    [KEYCODE_4]         = '$',
    [KEYCODE_5]         = '%',
    [KEYCODE_6]         = '^',
    [KEYCODE_7]         = '&',
    [KEYCODE_8]         = '*',
    [KEYCODE_9]         = '(',
    [KEYCODE_0]         = ')',
    [KEYCODE_MINUS]     = '_',
    [KEYCODE_EQUAL]     = '+',
    [KEYCODE_BACKSPACE] = '\b',

    [KEYCODE_TAB]       = '\t',
    [KEYCODE_Q]         = 'Q',
    [KEYCODE_W]         = 'W',
    [KEYCODE_E]         = 'E',
    [KEYCODE_R]         = 'R',
    [KEYCODE_T]         = 'T',
    [KEYCODE_Y]         = 'Y',
    [KEYCODE_U]         = 'U',
    [KEYCODE_I]         = 'I',
    [KEYCODE_O]         = 'O',
    [KEYCODE_P]         = 'P',
    [KEYCODE_LBRACKET]  = '{',
    [KEYCODE_RBRACKET]  = '}',
    [KEYCODE_BSLASH]    = '|',

    [KEYCODE_A]         = 'A',
    [KEYCODE_S]         = 'S',
    [KEYCODE_D]         = 'D',
    [KEYCODE_F]         = 'F',
    [KEYCODE_G]         = 'G',
    [KEYCODE_H]         = 'H',
    [KEYCODE_J]         = 'J',
    [KEYCODE_K]         = 'K',
    [KEYCODE_L]         = 'L',
    [KEYCODE_SEMICOLON] = ':',
    [KEYCODE_QUOT]      = '"',
    [KEYCODE_ENTER]     = '\n',

    [KEYCODE_Z]         = 'Z',
    [KEYCODE_X]         = 'X',
    [KEYCODE_C]         = 'C',
    [KEYCODE_V]         = 'V',
    [KEYCODE_B]         = 'B',
    [KEYCODE_N]         = 'N',
    [KEYCODE_M]         = 'M',
    [KEYCODE_COMMA]     = '<',
    [KEYCODE_PERIOD]    = '>',
    [KEYCODE_SLASH]     = '?',
    [KEYCODE_SPACE]     = ' ',

    [KEYCODE_PAD_SLASH] = '/',
    [KEYCODE_PAD_STAR]  = '*',
    [KEYCODE_PAD_7]     = '7',
    [KEYCODE_PAD_8]     = '8',
    [KEYCODE_PAD_9]     = '9',
    [KEYCODE_PAD_MINUS] = '-',
    [KEYCODE_PAD_4]     = '4',
    [KEYCODE_PAD_5]     = '5',
    [KEYCODE_PAD_6]     = '6',
    [KEYCODE_PAD_PLUS]  = '+',
    [KEYCODE_PAD_1]     = '1',
    [KEYCODE_PAD_2]     = '2',
    [KEYCODE_PAD_3]     = '3',
    [KEYCODE_PAD_0]     = '0',
    [KEYCODE_PAD_DOT]   = '.'
};

static char keycode_to_ascii(Keycode code) {
    bool capitalize = (modifiers.shift || toggles.caps_lock) && 
        !(modifiers.shift && toggles.caps_lock);
    if (code > KEYCODE_PAD_DOT) return '\0';
    if (capitalize) return upper_ascii[code];
    return lower_ascii[code];
}

static void set_key_flag(Keycode code, bool enabled) {
    uint8_t mask = 1 << (code & 0x111);
    
    if (enabled)
        pressed_keys[code >> 3] |= mask;
    else
        pressed_keys[code >> 3] &= ~mask;
}

static bool get_key_flag(Keycode code) {
    int mask = 1 << (code & 0x111);
    return (pressed_keys[code >> 3] & mask) != 0;
}

static Keycode scanset2_to_keycode() {
    if (scancode_byte_count == 1) {
        if (scancode_bytes[0] <= 0x83) return scancode_set_2[scancode_bytes[0]];
        return 0;
    }

    if (scancode_byte_count == 2 && scancode_bytes[0] == 0xF0) {
        if (scancode_bytes[1] <= 0x83) return scancode_set_2[scancode_bytes[1]];
        return 0;
    }

    if (scancode_byte_count == 2 && scancode_bytes[0] == 0xE0) {
        if (scancode_bytes[1] <= 0x7D) 
            return e_scancode_set_2[scancode_bytes[1]];
        return 0;
    }

    if (scancode_byte_count == 3 && scancode_bytes[0] == 0xE0 && 
        scancode_bytes[1] == 0xF0
    ) {
        if (scancode_bytes[2] <= 0x7D) 
            return e_scancode_set_2[scancode_bytes[2]];
        return 0;
    }

    if (scancode_byte_count == 4) {
        if (scancode_bytes[0] == 0xE0 && scancode_bytes[1] == 0x12 && 
            scancode_bytes[2] == 0xE0 && scancode_bytes[3] == 0x7C
        ) {
            return KEYCODE_PRINTSCR;
        }
        return 0;
    }

    if (scancode_byte_count == 6) {
        if (scancode_bytes[0] == 0xE1 && scancode_bytes[1] == 0xF0 && 
            scancode_bytes[2] == 0x7C && scancode_bytes[3] == 0xE1 && 
            scancode_bytes[4] == 0xF0 && scancode_bytes[5] == 0x12
        ) {
            return KEYCODE_PRINTSCR;
        }
        return 0;
    }

    if (scancode_byte_count == 8) return KEYCODE_PAUSE;
    return 0;
}

static void update_special_keys(Keycode code, bool released) {
    switch (code)
    {
    case KEYCODE_LSHIFT:
    case KEYCODE_RSHIFT:
        modifiers.shift = !released;
        return;
    case KEYCODE_LALT:
    case KEYCODE_RALT:
        modifiers.alt = !released;
        return;
    case KEYCODE_LCTRL:
    case KEYCODE_RCTRL:
        modifiers.control = !released;
        return;
    case KEYCODE_FN:
        modifiers.function = !released;
        return;
    case KEYCODE_LGUI:
    case KEYCODE_RGUI:
        modifiers.gui = !released;
        return;
    }

    if (released) return;
    switch (code)
    {
    case KEYCODE_CAPS:
        toggles.caps_lock = !toggles.caps_lock;
        return;
    case KEYCODE_NUMLOCK:
        toggles.num_lock = !toggles.num_lock;
        return;
    case KEYCODE_SCLLK:
        toggles.scroll_lock = !toggles.scroll_lock;
        return;
    }
}

/**
 * Called when the keyboard sends a scancode
*/
static void on_key_pressed(uint8_t scancode) {
    scancode_bytes[scancode_byte_count] = scancode;
    scancode_byte_count++;

    if (scancode == 0xF0)
        scancode = 0xF0;

    bool continue_scan = (
        (scancode == 0xE0) ||
        (scancode == 0xE1) ||
        (scancode == 0xF0) ||
        (scancode_byte_count == 2 && scancode_bytes[0] == 0xE0 
            && scancode == 0x12) ||
        (scancode_byte_count == 3 && scancode_bytes[0] == 0xE0 
            && scancode == 0x7C)
    );

    bool pause_scancode = true;
    for (int i=0; i<scancode_byte_count; i++) 
        pause_scancode &= scancode_bytes[i] == scancode_set_2_pause[i];
    continue_scan |= pause_scancode;

    if (!continue_scan) {
        Keycode keycode = scanset2_to_keycode();
        bool released = (scancode_byte_count > 1 && scancode_bytes[0] == 0xF0) 
            || (scancode_byte_count > 2 && scancode_bytes[1] == 0xF0);
        set_key_flag(keycode, released);
        update_special_keys(keycode, released);
        scancode_byte_count = 0;

        if (keypress_handler != NULL) {
            KeypressEvent *event = malloc(sizeof(KeypressEvent));
            event->ascii = keycode_to_ascii(keycode);
            event->code = keycode;
            event->released = released;
            event->modifiers.shift = modifiers.shift;
            event->modifiers.alt = modifiers.alt;
            event->modifiers.control = modifiers.control;
            event->modifiers.function = modifiers.function;
            event->modifiers.gui = modifiers.gui;
            event->toggles.caps_lock = toggles.caps_lock;
            event->toggles.num_lock = toggles.num_lock;
            event->toggles.scroll_lock = toggles.scroll_lock;
            add_event(keypress_handler, event);
        }
    }
}

/**
 * Add a command to the command queue and start executing it if it is the first
 * one
 * 
 * Parameters:
 *   code: The initial code
 *   has_sub: Whether or not there is a subcommand
 *   sub: The subcommand (only used if has_sub is true)
 *   has_response: Whether a response byte is expected
 *   on_complete: The function to call when the command completes
 * 
 * Returns:
 *   false if the command could not be added (queue full), true otherwise
*/
static bool add_command(
    KbdCmdCode code, bool has_sub, uint8_t sub, bool has_response,
    void (*on_complete)(bool, uint8_t)
) {
    if (queue_count == MAX_COMMANDS) return false;

    // Setup the command
    KbdCommand* cmd = &cmd_queue[(queue_min + queue_count) % MAX_COMMANDS];
    cmd->code = code;
    cmd->has_subcommand = has_sub;
    cmd->subcommand = sub;
    cmd->has_response = has_response;
    cmd->on_complete = on_complete;
    cmd->sub_sent = false;
    cmd->acknowledged = false;
    cmd->resend_count = 0;

    // Start the command if it is the first one
    if (++queue_count == 1) device->send_byte(code);
}

/**
 * Discards the current command and starts the next one if there is one
*/
static void next_command() {
    queue_min = (queue_min + 1) % MAX_COMMANDS;
    queue_count--;
    if (queue_count > 0) device->send_byte(cmd_queue[queue_min].code);
}

/**
 * Handler for the keyboard
*/
static void on_keyboard_send_byte(uint8_t byte) {
    // If there is no command then it should be a scancode
    if (queue_count == 0) {
        on_key_pressed(byte);
        return;
    }

    KbdCommand *cmd = &cmd_queue[queue_min];

    // Process a resend request
    if (byte == RESPONSE_RESEND) {
        uint8_t code = cmd->code;
        if (cmd->sub_sent) cmd->code = cmd->subcommand;

        cmd->resend_count++;

        // Resend the command if we have not errored too many times
        if (cmd->resend_count <= 3) {
            device->send_byte(code);
        } else {
            cmd->on_complete(false, 0);
            next_command();
        }
        return;
    }

    // Process an acknowledgement
    if (byte == RESPONSE_ACK) {
        // Send the subcommand if there is one
        if (cmd->has_subcommand && !cmd->sub_sent) {
            cmd->sub_sent = true;
            cmd->resend_count = 0;
            device->send_byte(cmd->subcommand);
        } else {
            // If the commands have been sent and there is no response then
            // done
            cmd->acknowledged = true;
            if (!cmd->has_response) {
                cmd->on_complete(true, 0);
                next_command();
            }
        }

        return;
    }

    // If the command was already acknowledged this should be a response byte
    if (cmd->acknowledged) {
        cmd->on_complete(true, byte);
        next_command();
    } else {
        // If the command was not acknowledged maybe this is a scancode
        on_key_pressed(byte);
    }
}

static void continue_init(bool success, uint8_t response) {
    if (!success) {
        printf("[KBD] ERROR Initialize Command Not Acknowledged");
        initialized = true;
        return;
    }

    switch (init_state)
    {
    case INIT_START:
        init_state = DISABLE_SCAN;
        add_command(CMD_DISABLE_SCAN, false, 0, false, continue_init);
        return;
    case DISABLE_SCAN:
        init_state = SET_SCAN_SET_3;
        add_command(CMD_SCAN_CODE_SET, true, 0x2, false, continue_init);
        return;
    case SET_SCAN_SET_3:
        init_state = GET_SCAN_SET;
        add_command(CMD_SCAN_CODE_SET, true, 0x0, true, continue_init);
        return;
    case GET_SCAN_SET:
        if (response != 2) {
            printf("  Error: Scancode Set 2 Not Supported\n");
            initialized = true;
            return;
        }
        printf("  Using Scancode Set: %d\n", response);
        scan_code_set = response;
        init_state = ENABLE_SCAN;
        add_command(CMD_ENABLE_SCAN, false, 0, false, continue_init);
        return;
    case ENABLE_SCAN:
        init_state = INITIALIZED;
        initialized = true;
        return;
    }
}

// Determine if a device is a keyboard
static bool is_keyboard(PS2_Device *device) {
    switch (device->type)
    {
    case AT_KBD:
    case MF2_KBD:
    case SHORT_KBD:
    case HOST_KBD_122_KEY:
    case KBD_122_KEY:
    case JAPANESE_G_KBD:
    case JAPANESE_P_KBD:
    case JAPANESE_A_KBD:
    case NCD_SUN_KBD:
        return true;
    default:
        return false;
    }
}

void kbd_initialize() {
    printf("Initializing Keyboard\n");
    device = ps2_get_port_1_device();
    if (!is_keyboard(device)) {
        device = ps2_get_port_2_device();
        if (!is_keyboard(device)) {
            device = NULL;
            printf("  No Keyboard Found\n");
            return;
        }
    }
    printf("  Found Keyboard Device\n");
    device->on_byte_recieved = on_keyboard_send_byte;

    initialized = false;
    init_state = INIT_START;
    modifiers.alt = false;
    modifiers.shift = false;
    modifiers.control = false;
    modifiers.gui = false;
    modifiers.function = false;
    toggles.caps_lock = false;
    toggles.scroll_lock = false;
    toggles.num_lock = false;
    
    continue_init(true, 0);

    while(!initialized);

    printf("Initialized Keyboard");
}

void register_handler(Event_Handler handler) {
    keypress_handler = handler;
}
