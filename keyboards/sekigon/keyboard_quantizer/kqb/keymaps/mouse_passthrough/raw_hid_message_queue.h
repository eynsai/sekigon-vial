#pragma once

#include QMK_KEYBOARD_H

void message_queue_push(const uint8_t* data);
void message_queue_pop(uint8_t* buffer);
uint8_t get_message_queue_length(void);
