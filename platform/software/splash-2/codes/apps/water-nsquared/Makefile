CFLAGS =  $(TARGET_CFLAGS)
CFLAGS += -I$(BUILDDIR)/
CFLAGS += -I$($(TARGET_SYSTEM_KSP_OS)_INCLUDE)
CFLAGS += -I$($(TARGET_SYSTEM_KSP_TASK)_INCLUDE)
CFLAGS += -I$($(TARGET_SYSTEM_SSP_CAL)_INCLUDE)
CFLAGS += -I$($(TARGET_SYSTEM_SSP_PAL)_INCLUDE)


SRC_H=    $(wildcard *.H)
SRC_C=    $(wildcard *.C)

OBJ_c=    $(SRC_C:%.C=%.c)
OBJ_h=    $(SRC_H:%.H=%.h)

OBJ=    $(OBJ_c:%.c=$(BUILDDIR)/%.o)
BASEDIR := $(INSTALL_BASE)/platform/software/splash-2/codes
MACRO_FILE := $(BASEDIR)/null_macros/c.m4.null.POSIX_DNA_OS
TARGET_M4 :=  m4 -s -Ulen -Uindex

default: $(OBJ_c) $(OBJ_h)
	mkdir -p Headers Sources
	mv *.h Headers; mv *.c Sources
	apes-compose -c
	apes-compose


#ifdef MAKEFILE_RULES
#include $(MAKEFILE_RULES)
#else
#include $(APES_HOME)/support/Makefile.rules
#endif

clean:
	rm -f bin/*
	rm -f Sources/*.c
	rm -f Headers/*.h
	rm -f waternsquared

app_clean :
	echo '[CLEAN...] $(APP_NAME)/$(APP_NAME_EXT) [h/c]'
	rm -f $(OBJ_c) $(OBJ_h)

%.h: %.H
	$(TARGET_M4) $(TARGET_M4FLAGS) $(MACRO_FILE) $< > $@

%.c: %.C
	$(TARGET_M4) $(TARGET_M4FLAGS) $(MACRO_FILE) $< > $@

