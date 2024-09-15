#include "raw_hid_message_queue.h"

#define QMK_RAW_HID_REPORT_SIZE 32

typedef struct raw_hid_message_t {
    unsigned char data[QMK_RAW_HID_REPORT_SIZE];
    struct raw_hid_message_t* next;
} raw_hid_message_t;

raw_hid_message_t* message_queue = NULL;
uint8_t message_queue_length = 0;

void message_queue_push(const uint8_t* data) {
    raw_hid_message_t* new_message = (raw_hid_message_t*)malloc(sizeof(raw_hid_message_t));
    if (new_message == NULL) {
        return;
    }
    if (message_queue_length == 255) {
        return;
    }
    message_queue_length++;
    memcpy(new_message->data, data, QMK_RAW_HID_REPORT_SIZE);
    new_message->next = NULL;
    if (message_queue == NULL) {
        message_queue = new_message;
    } else {
        raw_hid_message_t* current_message = message_queue;
        while (current_message->next != NULL) {
            current_message = current_message->next;
        }
        current_message->next = new_message;
    }
}

void message_queue_pop(uint8_t* buffer) {
    if (message_queue_length == 0) {
        return;
    }
    message_queue_length--;
    raw_hid_message_t* popped_message = message_queue;
    memcpy(buffer, popped_message->data, QMK_RAW_HID_REPORT_SIZE);
    message_queue = popped_message->next;
    free(popped_message);
}

uint8_t get_message_queue_length(void) {
    return message_queue_length;
}