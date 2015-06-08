HW           = $(RABBITS_DIR)
SYSTEMC      = $(RABBITS_SYSTEMC)
QEMU         = $(HW)/rabbits/libs
#QEMU         = libs

COMPONENTS   = rabbits/components
PLATFORMS    = platforms
SRC_BASE_DIR = $(HW)
HWETRACE_LIBDIR = $(DECOPUS)/trace/lib
#HWETRACE_LIBDIR = libs
SUB_DIRS     = $(COMPONENTS)/abstract_noc $(COMPONENTS)/mem    			 \
			   $(COMPONENTS)/master_device $(COMPONENTS)/slave_device    \
			   $(COMPONENTS)/qemu_wrapper $(COMPONENTS)/sl_tty           \
			   $(COMPONENTS)/sl_timer $(COMPONENTS)/sl_mailbox           \
			   $(COMPONENTS)/aicu $(COMPONENTS)/framebuffer              \
			   $(COMPONENTS)/system/app_loader				             \
			   $(COMPONENTS)/sl_blockdevice   				             \
			   $(COMPONENTS)/generic_subsystem				             \
			   $(COMPONENTS)/generic_SMSMP_subsystem                     \
               $(PLATFORMS)/bunny


CC           = g++
OPT          = -O3 
#DEBUG        = -g -fprofile-arcs -ftest-coverage
#DEBUG        = -g --coverage
DEBUG        = -g 
OTHER        = -Wall -Werror -std=gnu++11
TRACE        = -DTRACE_EVENT_ENABLED -DTRACE_EVENT_BLOCK_CYCLES  -DRABBITS_TRACE_EVENT_CACHE
CFLAGS       = $(DEBUG) $(OTHER) $(OPT) $(TRACE)

ARCH         = $(shell uname -m)
TARGET_ARCH  = linux

ifeq ($(ARCH), x86_64)
	SC_LIBDIR = $(SYSTEMC)/lib-$(TARGET_ARCH)64
else
	SC_LIBDIR = $(SYSTEMC)/lib-$(TARGET_ARCH)
endif

LIBDIR       = -L$(SC_LIBDIR) -L$(HWETRACE_LIBDIR)
LIBS         = -ldl -lpthread -lelf -lsystemc -lhwetrace -lrt 
RLIBDIR      = -Wl,-rpath $(SC_LIBDIR) -Wl,-rpath $(QEMU) -Wl,-rpath $(HWETRACE_LIBDIR) 



INCDIR += -I$(SYSTEMC)/include 
INCDIR += $(foreach SUB_DIR, $(SUB_DIRS), $(addprefix -I$(SRC_BASE_DIR)/, $(SUB_DIR)))
INCDIR += -I$(DECOPUS)/trace/include

MODULE       = run
EXE          = $(MODULE).x

BUILD_DIR    = ./objs
define FIND_SRCS
$(addprefix $(1)/, $(notdir $(wildcard $(SRC_BASE_DIR)/$(1)/*.cpp)))
endef
TMPSRCS=$(foreach SUB_DIR, $(SUB_DIRS), $(call FIND_SRCS,$(SUB_DIR)))
SRCS=$(addprefix $(SRC_BASE_DIR)/, $(TMPSRCS))
OBJS=$(addprefix $(BUILD_DIR)/, $(TMPSRCS:.cpp=.o))
DEPS=$(OBJS:.o=.d)

$(shell mkdir -p $(addprefix $(BUILD_DIR)/, $(SUB_DIRS)))

.SUFFIXES: .cc .cpp .o .x
.PHONY: clean all graphs verif

all: $(EXE)

$(EXE): $(OBJS)
	@echo Building $@ ...
	@$(CC) $(CFLAGS) $(LIBDIR) $(RLIBDIR) -o $@ $(OBJS) $(LIBS) 2>&1 | c++filt

$(BUILD_DIR)/%.o:$(SRC_BASE_DIR)/%.cpp
	@echo "Compiling $(notdir $<) ..."
	@rm -fr $(addsuffix .d, $(basename $@))
	@$(CC) $(CFLAGS) $(INCDIR) -o $@ -c $<
	@echo -n "$(dir $@)" > $(addsuffix .d, $(basename $@))
	@$(CC) $(CFLAGS) $(INCDIR) -M $< >> $(addsuffix .d, $(basename $@))

link: clean_exe all

clean_exe:
	@rm -f $(EXE)

clean:
	@echo Cleaning ...
	@rm -f $(EXE)
	@rm -f $(addsuffix /*.o, $(addprefix $(BUILD_DIR)/, $(SUB_DIRS)))
	@rm -f $(addsuffix /*.d, $(addprefix $(BUILD_DIR)/, $(SUB_DIRS)))

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
