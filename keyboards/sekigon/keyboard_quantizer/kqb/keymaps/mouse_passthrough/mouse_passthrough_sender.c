#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include "raw_hid_message_queue.h"
#include "mouse_passthrough_sender.h"

#define HUB_CONNECTION_ATTEMPT_INTERVAL 4000
#define HUB_CONNECTION_EXPIRY_INTERVAL 5000

#define RAW_HID_HUB_COMMAND_ID 0x27
#define QMK_RAW_HID_REPORT_SIZE 32
#define N_UNIQUE_DEVICE_IDS 255
#define DEVICE_ID_UNASSIGNED N_UNIQUE_DEVICE_IDS
#define DEVICE_ID_HUB N_UNIQUE_DEVICE_IDS
#define MAX_REGISTERED_DEVICES 30

enum mouse_passthrough_sender_states {
    MOUSE_PASSTHROUGH_DISCONNECTED = 0,
    MOUSE_PASSTHROUGH_HUB_CONNECTED,
    MOUSE_PASSTHROUGH_KEYBOARD_CONNECTED,
};

uint8_t state;
uint8_t self_device_id;
uint8_t other_device_ids[MAX_REGISTERED_DEVICES - 1];
uint8_t keyboard_device_id;
uint32_t last_connection_attempt_time;
uint32_t last_connection_success_time;

void mouse_passthrough_sender_matrix_scan_task(void) {
    uint8_t report[32] = {0};

    if (timer_elapsed32(last_connection_success_time) > HUB_CONNECTION_EXPIRY_INTERVAL) {
        state = MOUSE_PASSTHROUGH_DISCONNECTED;
    }

    if (timer_elapsed32(last_connection_attempt_time) > HUB_CONNECTION_ATTEMPT_INTERVAL) {
        last_connection_attempt_time = timer_read32();
        // send a registration report
        report[0] = RAW_HID_HUB_COMMAND_ID;
        report[1] = 0xFF;
        report[2] = 0x01;
        message_queue_push(report);
        
        if (state == MOUSE_PASSTHROUGH_HUB_CONNECTED) {
            // handshake step 1/4: mouse broadcasts to all devices
            for (int i = 0; i < MAX_REGISTERED_DEVICES - 1; i++) {
                if (other_device_ids[i] != DEVICE_ID_UNASSIGNED) {
                    report[0] = RAW_HID_HUB_COMMAND_ID;
                    report[1] = other_device_ids[i];
                    report[2] = 13;
                    message_queue_push(report);
                }
            }
        }
    }

    if (get_message_queue_length() != 0) {
        message_queue_pop(report);
        report[QMK_RAW_HID_REPORT_SIZE - 1] = get_message_queue_length();
        raw_hid_send(report, QMK_RAW_HID_REPORT_SIZE);
    }
}

bool mouse_passthrough_sender_raw_hid_receive_task(uint8_t* data) {

    if (data[0] != RAW_HID_HUB_COMMAND_ID) {
        return true;
    }

    last_connection_success_time = timer_read32();
    if (state == MOUSE_PASSTHROUGH_DISCONNECTED) {
        state = MOUSE_PASSTHROUGH_HUB_CONNECTED;
    }

    if (data[1] == DEVICE_ID_HUB) {
        self_device_id = data[2];
        memcpy(other_device_ids, data + 3, MAX_REGISTERED_DEVICES - 1);
        if (state == MOUSE_PASSTHROUGH_KEYBOARD_CONNECTED) {
            // see if we're still connected
            bool found = false;
            for (int i = 3; i < QMK_RAW_HID_REPORT_SIZE; i++) {
                if (data[i] == keyboard_device_id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                state = MOUSE_PASSTHROUGH_HUB_CONNECTED;
            }
        }
        
    } else if (state == MOUSE_PASSTHROUGH_HUB_CONNECTED && data[2] == 26) {
        // handshake step 3/4: mouse responds to first keyboard it hears from
        state = MOUSE_PASSTHROUGH_KEYBOARD_CONNECTED;
        keyboard_device_id = data[1];
        uint8_t report[32] = {0};
        report[0] = RAW_HID_HUB_COMMAND_ID;
        report[1] = keyboard_device_id;
        report[2] = 39;
        message_queue_push(report);
    }

    return false;
}

bool mouse_passthrough_sender_pointing_device_task(uint8_t buttons, int16_t x, int16_t y, int16_t v, int16_t h) {
    if (state != MOUSE_PASSTHROUGH_KEYBOARD_CONNECTED) {
        return true;
    }
    uint8_t report[32] = {0};
    report[0] = RAW_HID_HUB_COMMAND_ID;
    report[1] = keyboard_device_id;
    report[2] = buttons;
    report[3] = (x >> 8) & 0xFF;
    report[4] = x & 0xFF;
    report[5] = (y >> 8) & 0xFF;
    report[6] = y & 0xFF;
    report[7] = (v >> 8) & 0xFF;
    report[8] = v & 0xFF;
    report[9] = (h >> 8) & 0xFF;
    report[10] = h & 0xFF;
    message_queue_push(report);
    return false;
}