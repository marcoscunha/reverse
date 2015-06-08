export PROJ_BASE=$(PWD)

CFLAGS=-g -std=c99 -Wall -Werror -O3 -fpic -I../../include
LDFLAGS=

LOADER=sherlock

SRC=loader.c
OBJ=$(SRC:.c=.o)

all: $(OBJ)

%.o: %.c
	@echo "Compiling : $@"
	@gcc $< $(CFLAGS) -o $(LOADER)

clean:
	@echo "Cleaning all objects"
	@rm -rf *.o

install:
	@echo "Copying $(LOADER)"
	@mv $(LOADER) $(PROJ_BASE)

mrproper: clean

.PHONY: clean mrproper core all install

