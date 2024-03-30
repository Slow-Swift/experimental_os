#include "ps2.h"

#include <arch/i686/acpi.h>
#include <arch/i686/io.h>
#include <arch/i686/irq.h>
#include <debug.h>
#include <stdbool.h>
#include <stdio.h>

static enum {
    PORT_DATA   = 0x60,
    PORT_STATUS = 0x64,
    PORT_CMD    = 0x64,

    PORT_1      = 0x01,
    PORT_2      = 0x02,
} PS2_PORTS;

static enum {
    CMD_READ_RAM = 0x20,
    CMD_WRITE_RAM = 0x60,
    CMD_DISABLE_PORT_2 = 0xA7,
    CMD_ENABLE_PORT_2 = 0xA8,
    CMD_TEST_PORT_2 = 0xA9,
    CMD_TEST_CONTROLLER = 0xAA,
    CMD_TEST_PORT_1 = 0xAB,
    CMD_DUMP = 0xAC,
    CMD_DISABLE_PORT_1 = 0xAD,
    CMD_ENABLE_PORT_1 = 0xAE,
    CMD_READ_INPUT = 0xC0,
    CMD_COPY_INPUT_LOW = 0xC1,
    CMD_COPY_INPUT_HIGH = 0xC2,
    CMD_READ_OUTPUT = 0xD0,
    CMD_WRITE_OUTPUT = 0xD1,
    CMD_WRITE_OUTPUT_1 = 0xD2,
    CMD_WRITE_OUTPUT_2 = 0xD3,
    CMD_WRITE_INPUT_2 = 0xD4,
    CMD_PULSE_LOW = 0xF0
} PS2_COMMANDS;

static enum {
    DEVICE_IDENTIFY = 0xF2,
    DEVICE_ENABLE_SCAN = 0xF4,
    DEVICE_DISABLE_SCAN = 0xF5,
    DEVICE_RESET = 0xFF,
    
    DEVICE_ACK = 0xFA,
    DEVICE_FAIL = 0xFC
} DEVICE_COMMANDS;

static enum {
    ERROR_TIMEOUT = -0x01,
    ERROR_NOT_PRESENT = -0x02,
    ERROR_FAILURE = -0x03,
    ERROR_SCAN_SET_FAIL = -0x04,
} ERROR_CODES;

char* ps_2_device_type_names[] = {
    "None",
    "Unkown",
    "AT Keyboard",
    "PS/2 Mouse",
    "Scroll Wheel Mouse",
    "5-Button Mouse",
    "MF2 Keyboard",
    "Short Keyboard",
    "122-Key Host Keyboard",
    "122-Key Keyboard",
    "Japanese \"G\" Keyboard",
    "Japanese \"P\" Keyboard",
    "Japanese \"A\" Keyboard",
    "NCD Sun Keyboard"
};

static bool is_present = false;
static bool is_initialized = false;
static bool is_dual_channel = false;
static bool channel_1_ok = false;
static bool channel_2_ok = false;

static PS2_Device port1_device;
static PS2_Device port2_device;

static const int WAIT_TIME = 0xFFFF;

static int wait_ps2_cmd() {
    int wait_timer = WAIT_TIME;
    io_wait();
    while(((in_byte(PORT_STATUS) & 0x2) != 0) && (--wait_timer != 0));
    if ((in_byte(PORT_STATUS) & 0x2) != 0) return ERROR_TIMEOUT;
    return 0;
}

static bool wait_ps2_data() {
    int wait_timer = WAIT_TIME;
    io_wait();
    while((in_byte(PORT_STATUS) & 0x1) == 0 && (--wait_timer != 0));
    if ((in_byte(PORT_STATUS) & 0x1) == 0) return ERROR_TIMEOUT;
    return 0;
}

static bool detect_ps2() {
    printf("Detecting Presence of PS/2 Controller...\n");
    FADT* fadt = get_fadt();
    int acpi_version = fadt->header.revision;

    // Assume present if ACPI version is 1
    if (acpi_version == 1)
        is_present = true;
    else if (acpi_version > 1) {
        is_present = (fadt->boot_architecture_flags & 0x2) != 0;
        printf("Boot Architecture Flags: %#x", fadt->boot_architecture_flags);
    }

    if (is_present)
        printf("  PS/2 Controller [Present]\n");
    else
        printf("  PS/2 Controller [Missing]\n");

    return is_present;
}

static void enable_port(int port) {
    int cmd = port == PORT_1 ? 
        CMD_ENABLE_PORT_1 : CMD_ENABLE_PORT_2;

    wait_ps2_cmd();
    out_byte(PORT_CMD, cmd);
}

static void disable_port(int port) {
    int cmd = port == PORT_1 ? 
        CMD_DISABLE_PORT_1 : CMD_DISABLE_PORT_2;

    wait_ps2_cmd();
    out_byte(PORT_CMD, cmd);
}

static bool test_port(int port) {
    int cmd = port == PORT_1 ? CMD_TEST_PORT_1 : CMD_TEST_PORT_2;

    wait_ps2_cmd();
    out_byte(PORT_CMD, cmd);
    wait_ps2_data();
    return in_byte(PORT_DATA) == 0;
}

static uint8_t get_configuration() {
    wait_ps2_cmd();
    out_byte(PORT_CMD, CMD_READ_RAM);
    wait_ps2_data();
    return in_byte(PORT_DATA);
}

static void write_configuration(uint8_t configuration) {
    wait_ps2_cmd();
    out_byte(PORT_CMD, CMD_WRITE_RAM);
    wait_ps2_cmd();
    out_byte(PORT_DATA, configuration);
}

static uint8_t configure() {
    uint8_t configuration = get_configuration();
    configuration &= ~(1);
    configuration &= ~(1 << 1);
    configuration &= ~(1 << 6);
    write_configuration(configuration);

    return configuration;
}

static bool perform_self_test() {
    wait_ps2_cmd();
    out_byte(PORT_CMD, CMD_TEST_CONTROLLER);
    wait_ps2_data();
    return in_byte(PORT_DATA) == 0x55;
}

static void enable_availiable_channels() {
    if(channel_1_ok) enable_port(PORT_1);
    if(channel_2_ok) enable_port(PORT_2);
} 

static void enable_irqs() {
    uint8_t configuration = get_configuration();
    configuration |= 0x01 & channel_1_ok;
    configuration |= 0x02 & channel_2_ok;
    write_configuration(configuration);
}

static int send_byte(int port, uint8_t data) {
    if (port == PORT_2) {
        if (wait_ps2_cmd() < 0) return ERROR_TIMEOUT;
        out_byte(PORT_CMD, CMD_WRITE_INPUT_2);
    }

    if (wait_ps2_cmd() < 0) return ERROR_TIMEOUT;
    out_byte(PORT_DATA, data);
    return 0;
}

static int send_byte_port_1(uint8_t data) {
    send_byte(PORT_1, data);
}

static int send_byte_port_2(uint8_t data) {
    send_byte(PORT_2, data);
}

static int detect_device(PS2_Device *device) {
    device->present = false;

    send_byte(device->port, DEVICE_RESET);
    if (wait_ps2_data() < 0) return ERROR_TIMEOUT;
    if (in_byte(PORT_DATA) != DEVICE_ACK) return ERROR_FAILURE;
    if (wait_ps2_data() >= 0) {
        in_byte(PORT_DATA);
    }
    if (wait_ps2_data() >= 0) {
        in_byte(PORT_DATA);
    }
    device->present = true;

    return 0;
}

static int set_scanning(PS2_Device *device, bool scan) {
    if (!device->present) return ERROR_NOT_PRESENT;
    int cmd = scan ? DEVICE_ENABLE_SCAN : DEVICE_DISABLE_SCAN;

    wait_ps2_cmd();
    send_byte(device->port, cmd);
    if(wait_ps2_data() < 0) return ERROR_NOT_PRESENT;
    if (in_byte(PORT_DATA) != DEVICE_ACK) return ERROR_FAILURE;
    return 0;
}

static PS2_Device_Type determine_device_type(uint8_t byte_1, uint8_t byte_2) {
        
    switch (byte_1) {
    case 0x00: return PS_2_MOUSE;
    case 0x03: return SCROLL_MOUSE;
    case 0x04: return BUTTON_5_MOUSE;
    }

    if (byte_1 == 0xAB) {
        switch (byte_2) {
        case 0x83:
        case 0xC1: return MF2_KBD;
        case 0x84: return SHORT_KBD;
        case 0x85: return HOST_KBD_122_KEY;
        case 0x86: return KBD_122_KEY;
        case 0x90: return JAPANESE_G_KBD;
        case 0x91: return JAPANESE_P_KBD;
        case 0x92: return JAPANESE_A_KBD;
        default: return UNKOWN;
        }
    } else if (byte_1 == 0xAC && byte_2 == 0xA1) {
        return NCD_SUN_KBD;
    } else {
        return UNKOWN;
    }

}

static int initialize_device(PS2_Device *device) {
    if (!device->present) return ERROR_NOT_PRESENT;

    // in_byte(PORT_DATA);

    if (wait_ps2_cmd() < 0) return ERROR_TIMEOUT;
    send_byte(device->port, DEVICE_IDENTIFY);
    if (wait_ps2_data() < 0) return ERROR_TIMEOUT;
    if (in_byte(PORT_DATA) != DEVICE_ACK) return ERROR_FAILURE;

    uint8_t byte_1, byte_2;
    if (wait_ps2_data() >= 0)
        byte_1 = in_byte(PORT_DATA);

    if (wait_ps2_data() >= 0)
        byte_2 = in_byte(PORT_DATA);

    device->type = determine_device_type(byte_1, byte_2);

    printf("    Device Identification: %#x, %#x\n", byte_1, byte_2);
    printf("    Device Type: %s\n", ps_2_device_type_names[device->type]);

    return 0;
}

static void initialize_devices() {
    port1_device = (PS2_Device) {
        .port = PORT_1,
        .present = false,
        .type = NONE,
        .send_byte = send_byte_port_1,
        .on_byte_recieved = NULL
    };

    port2_device = (PS2_Device) {
        .port = PORT_2,
        .present = false,
        .type = NONE, 
        .send_byte = send_byte_port_2,
        .on_byte_recieved = NULL
    };

    printf("  Detecting Devices...\n");
    if(detect_device(&port1_device) >= 0) 
        printf("    Device detected on port 1\n");
    if(detect_device(&port2_device) >= 0) 
        printf("    Device detected on port 2\n");

    printf("  Initializing Devices...\n");
    set_scanning(&port1_device, false);
    set_scanning(&port2_device, false);

    initialize_device(&port1_device);
    initialize_device(&port2_device);

    set_scanning(&port1_device, true);
    set_scanning(&port2_device, true);
}

static void port_1_interrupt(Registers* regs) {
    uint8_t byte = in_byte(PORT_DATA);
    if (port1_device.on_byte_recieved != NULL)
        port1_device.on_byte_recieved(byte);
}

static void port_2_interrupt(Registers* regs) {
    uint8_t byte = in_byte(PORT_DATA);
    if (port2_device.on_byte_recieved != NULL)
        port2_device.on_byte_recieved(byte);
}

PS2_Device* ps2_get_port_1_device() {
    return &port1_device;
}

PS2_Device* ps2_get_port_2_device() {
    return &port2_device;
}

bool ps2_configured_ok() {
    return is_present && is_initialized;
}

void ps2_initialize() {
    is_present = false;
    is_initialized = false;

    if (!detect_ps2()) return;

    printf("Intializing PS/2 Controller...\n");

    // Disable ports
    disable_port(PORT_1);
    disable_port(PORT_2);
    printf("  Disabled PS/2 Ports\n");

    // Flush the ouput buffer
    in_byte(PORT_DATA);
    printf("  Flushed PS/2 Output Buffer\n");

    // Change the configuration byte
    uint8_t configuration = configure();
    is_dual_channel = (configuration & (1 << 5)) != 0;
    printf("  Configured PS/2\n");

    // Perform the self test
    printf("  Performing PS/2 Self Test...\n");
    if (perform_self_test()) {
        printf("    Self Test [OK]\n");
    } else {
        printf("    Self Test [ERROR]\n");
        printf("  PS/2 Initialization [FAILED]\n");
        return;
    }

    // Rewrite the configuration byte in case the self test modfied it
    write_configuration(configuration);

    // Check that is actually is a dual channel
    printf("  Detecting Channels...\n");
    if (is_dual_channel) {
        enable_port(PORT_2);

        // Check configuration says port 2 enabled
        configuration = get_configuration();
        is_dual_channel == (configuration & (1 << 5)) == 0;
        disable_port(PORT_2);
    }

    if (is_dual_channel)
        printf("    Dected Dual Channel\n");
    else
        printf("    Detected Single Channel\n");

    // Test Channels
    printf("  Testing Channels...\n");
    channel_1_ok = test_port(PORT_1);
    channel_2_ok = test_port(PORT_2);
    printf("    Channel 1 [%s]\n", channel_1_ok ? "OK" : "ERROR");
    printf("    Channel 2 [%s]\n", channel_2_ok ? "OK" : "ERROR");

    enable_availiable_channels();
    printf("  Enabled Availiable Channels\n");

    initialize_devices();

    irq_register_handler(1, port_1_interrupt);
    irq_register_handler(12, port_2_interrupt);

    enable_irqs();

    is_initialized = true;
    printf("Initialized PS/2\n");
}