ENABLE_DEBUG ?=

DEVICE          = stm32f103cB
OPENCM3_DIR     = ../libopencm3

ifneq ($(V), 1)
MAKEFLAGS += --no-print-dir
Q := @
endif

OPT_FLAGS ?= -O2

LDFLAGS += $(OPT_FLAGS)

CFLAGS          += $(OPT_FLAGS) -ggdb3 -Wall -Wextra -Wno-char-subscripts -std=gnu99 -MD 
CPPFLAGS	+= -MD
LDFLAGS         += -static -nostartfiles  --specs=nano.specs -Wl,-Map=mapfile -Wl,-print-memory-usage
LDLIBS          += -Wl,--start-group -lc -lgcc  -Wl,--end-group
LSCRIPT = 

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/gcc-config.mk

ifeq ($(ENABLE_DEBUG), 1)
CFLAGS += -DENABLE_DEBUG
endif

SRC =			\
	main.c		\
	platform.c	\
	cdcacm.c	\
	usbuart.c	

OPT_FLAGS = -O2

ifeq ($(ENABLE_DEBUG), 1)
LDFLAGS += --specs=rdimon.specs
else
LDFLAGS += --specs=nosys.specs
endif

all:	version.h  pill_serial.elf 	pill_serial.bin


host_clean:
	-$(Q)$(RM) pill_serial.elf
	-$(Q)$(RM) pill_serial.bin

OBJS = $(SRC:.c=.o)

.PHONY:	clean host_clean FORCE

clean:	host_clean
	$(Q)echo "  CLEAN"
	-$(Q)$(RM) -f *.o *.d *~ *.elf *.bin $(HOSTFILES)
	-$(Q)$(RM) -f mapfile version.h


version.h: FORCE
	$(Q)echo "  GIT     version.h"
	$(Q)echo "#define FIRMWARE_VERSION \"`git describe --always --dirty`\"" > $@

-include *.d

include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/gcc-rules.mk

