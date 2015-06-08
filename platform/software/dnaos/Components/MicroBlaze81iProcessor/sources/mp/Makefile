CFLAGS  =  $(TARGET_CFLAGS)
CFLAGS += -I$(SYSTEM_SSP_CAL_XILINX_MICROBLAZE_81I_INCLUDE)
CFLAGS += -I$($(TARGET_SYSTEM_SSP_PAL)_INCLUDE)

SRC=  $(wildcard *.c) 
ASM=  $(wildcard *.S) 
OBJ=  $(SRC:%.c=$(BUILDDIR)/%.o) 
OBJ+= $(ASM:%.S=$(BUILDDIR)/%.o) 

default: apes_objects 

clean : apes_clean

ifdef MAKEFILE_RULES
include $(MAKEFILE_RULES)
else
include $(APES_HOME)/support/Makefile.rules
endif
