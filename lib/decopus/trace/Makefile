
all: print check nop trace order

HWETRACE=.

HWE_HANDLER_NOCRY=1
include makefile.handler

BIN_DIR=$(HWETRACE)/bin
LIB_BASE=$(HWETRACE)/lib

HWE_OBJS_:= hwetrace_write.o hwetrace_check.o hwetrace_print.o hwe_handle_nop.o hwetrace_order.o

HWE_OBJS:= $(addprefix $(HWETRACE_OBJ_DIR)/, $(HWE_OBJS_))


$(HWE_OBJS): $(HWE_UTILS_INC) $(HWE_EVENTS_INC) $(HWE_HANDLER_INC) Makefile

hwetrace_check.o: HWETRACE_CFLAGS+= -DHWE_HANDLE_REPORT=4

$(HWE_OBJS): $(HWETRACE_OBJ_DIR)/%.o: $(HWETRACE_SRC_DIR)/%.c
	@echo "Compiling $@"
	@mkdir -p $(HWETRACE_OBJ_DIR)
	@gcc -c -o $@ $< $(HWETRACE_CFLAGS)

nop: HWE_HANDLER_LIB_DIR=$(LIB_BASE).nop
nop: HWE_HANDLER_OBJS=$(HWETRACE_OBJ_DIR)/hwe_handle_nop.o

trace: HWE_HANDLER_LIB_DIR=$(LIB_BASE).trace
trace: HWE_HANDLER_OBJS=$(HWETRACE_OBJ_DIR)/hwetrace_write.o

check: HWE_HANDLER_BIN_DIR=$(BIN_DIR)
check: HWE_HANDLER_BIN_NAME=hwetrace_check
check: HWE_HANDLER_OBJS=$(HWETRACE_OBJ_DIR)/hwetrace_check.o

order: HWE_HANDLER_BIN_DIR=$(BIN_DIR)
order: HWE_HANDLER_BIN_NAME=hwetrace_order
order: HWE_HANDLER_OBJS=$(HWETRACE_OBJ_DIR)/hwetrace_order.o

export HWETRACE_CFLAGS_ADD:=
export HWE_HANDLER_BIN_DIR:=
export HWE_HANDLER_BIN_NAME:=
export HWE_HANDLER_LIB_DIR:=
export HWE_HANDLER_OBJS:=

order nop trace check debug:
	@echo "Making $@ handler"
	@make hwetrace_handler

print: $(BIN_DIR)/hwetrace_print

$(BIN_DIR)/hwetrace_print: $(HWETRACE_OBJ_DIR)/hwetrace_print.o
	@echo "Building $@"
	@mkdir -p $(BIN_DIR)
	@gcc -o $@ $<

clean: 
	rm -rf $(HWETRACE_OBJ_DIR)

mrproper: clean
	rm -rf $(BIN_DIR) $(LIB_BASE).nop* $(LIB_BASE).trace*

.PHONY: all print order check nop trace clean mrproper

