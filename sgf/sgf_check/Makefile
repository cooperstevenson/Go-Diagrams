# Makefile for SGFC
# Copyright (C) 1996-2003 by Arno Hollosi
# (see 'main.c' for more copyright information)


# System configuration
# SHELL = /bin/bash

# System environment
CC = gcc

CODEGEN = 

OPTIMIZATION = -O1

#OPTIONS = -pedantic -W -Wimplicit -Wreturn-type -Wswitch -Wformat\
#	  -Wuninitialized -Wparentheses -Wpointer-arith -Wbad-function-cast\
#	  -Wcast-qual -Wcast-align -Waggregate-return -Wstrict-prototypes\
#	  -Wnested-externs -Wshadow -Wchar-subscripts -ansi

OPTIONS = 


CFLAGS = $(CODEGEN) $(OPTIMIZATION) $(OPTIONS)


LIB = 

OBJ = execute.o properties.o parse.o parse2.o strict.o\
	load.o util.o main.o save.o gameinfo.o

ARCHIVE = 

sgfc: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(ARCHIVE) $(LIB)

all: clean sgfc

clean:
	rm -f $(OBJ) sgfc


execute.o: execute.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c execute.c

properties.o: properties.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c properties.c

parse.o: parse.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c parse.c

parse2.o: parse2.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c parse2.c

strict.o: strict.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c strict.c

load.o: load.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c load.c

util.o: util.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c util.c

main.o: main.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c main.c

save.o: save.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c save.c

gameinfo.o: gameinfo.c all.h protos.h 
	$(CC) $(CFLAGS) $(INCL) -c gameinfo.c

#END OF FILE#
