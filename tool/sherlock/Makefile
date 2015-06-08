export PROJ_BASE=$(PWD)

#HWETRACE=/home/cunha/devel/TIMA/Decopus/trace

#Compilaton Options
#CFLAGS=-g -std=c99 -Wall -Werror -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -O3 -DGDB_ENABLED
#LDFLAGS=-Llib -lanalysis
#RLIBDIR=-Wl,-rpath lib
OPT=-O3

#Directories
SRC=src
OBJ=obj
LIB=lib
BIN=bin

LOADER=loader
# Plugins description
# delorean - reverse debugger
# watson - coherence cache analysis
PLUGINS=delorean watson
#PLUGINS=watson 


# Application name
PROJ=sherlock 

all: tags $(BIN) $(OBJ) $(LIB) $(PROJ) install

$(PROJ): $(LOADER) $(PLUGINS)

$(LOADER):
	@echo "=================="
	@echo " Compiling $@"
	@echo "=================="
	@make -s -C src/$@

$(PLUGINS): clean
	@echo "=================="
	@echo " Compiling $@"
	@echo "=================="
	@make -s -C src/$@ OPT=$(OPT)

$(BIN) $(OBJ) $(LIB):
	@mkdir $@

clean:  
	@make -s -C src/$(LOADER)/ clean 
	@make -s -C src/delorean/ clean 
	@make -s -C src/watson/ clean 
	@rm -rf $(OBJ)/* 
	@rm -rf $(PROJ)

install:
	@echo "=================="
	@echo " Installing"
	@echo "=================="
	@make -s -C src/loader install
#	@make -s -C src/delorean install 
	@make -s -C src/watson install 


tags: $(BIN)
	@ctags-exuberant -R . 

mrproper: clean
	rm -rf $(PROJ) $(BIN) $(LIB) $(OBJ)

.PHONY: clean mrproper all install 

