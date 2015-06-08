TARGET = BARNES
ALLC = code.c code_io.c load.c grav.c getparam.c util.c

all : $(ALLC)
	mv *.c Sources; mv *.h Headers	
	apes-compose --clean
	apes-compose

include ../../Makefile.config

clean:
	rm -f bin/*
	rm -f Sources/*.c
	rm -f Headers/*.h
	rm -f barnes


stdinc.h: code.h defs.h util.h vectmath.h load.h code_io.h grav.h getparam.h stdinc.H 
code.c: code.C stdinc.h
code_io.c: code_io.C stdinc.h
getparam.c: getparam.C stdinc.h
grav.c: grav.C stdinc.h
load.c: load.C stdinc.h
util.c: util.C stdinc.h

