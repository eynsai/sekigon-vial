VIA_ENABLE = yes
VIAL_ENABLE = yes
VIAL_INSECURE = no
SRC += ../quantizer_mouse.c

USERSPACE_DIR = $(TOP_DIR)/../qmk_firmware/users/eynsai
SRC += $(USERSPACE_DIR)/mouse_passthrough_sender.c
VPATH += $(USERSPACE_DIR)

LTO_ENABLE = yes
OPT = 3  # -O3 optimization