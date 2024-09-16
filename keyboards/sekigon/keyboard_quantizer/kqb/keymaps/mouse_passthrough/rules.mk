VIA_ENABLE = yes
VIAL_ENABLE = yes
VIAL_INSECURE = no

SRC += ../quantizer_mouse.c
SRC += raw_hid_message_queue.c
SRC += mouse_passthrough_sender.c

LTO_ENABLE = yes
OPT = 3  # -O3 optimization