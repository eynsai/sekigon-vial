#pragma once

#define HUB_CONNECTION_ATTEMPT_INTERVAL 4000
#define HUB_CONNECTION_EXPIRY_INTERVAL 5000

#define RAW_HID_HUB_COMMAND_ID 0x27
#define QMK_RAW_HID_REPORT_SIZE 32
#define N_UNIQUE_DEVICE_IDS 255
#define DEVICE_ID_UNASSIGNED N_UNIQUE_DEVICE_IDS
#define DEVICE_ID_HUB N_UNIQUE_DEVICE_IDS
#define MAX_REGISTERED_DEVICES 30

enum mouse_passthrough_states {
    MOUSE_PASSTHROUGH_DISCONNECTED = 0,
    MOUSE_PASSTHROUGH_HUB_CONNECTED,
    MOUSE_PASSTHROUGH_TARGET_CONNECTED,
};

typedef struct passthrough_payload_t {
    uint8_t buttons;
    int16_t x;
    int16_t y;
    int16_t v;
    int16_t h;
} PACKED passthrough_payload_t;