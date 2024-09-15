#pragma once

void mouse_passthrough_sender_matrix_scan_task(void);
bool mouse_passthrough_sender_raw_hid_receive_task(uint8_t* data);
bool mouse_passthrough_sender_pointing_device_task(uint8_t buttons, int16_t x, int16_t y, int16_t h, int16_t v);