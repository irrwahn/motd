##################################
##  Makefile for project: motd  ##
##################################

PROJECT := motd

CC      ?= gcc
INCS    :=
CFLAGS  := $(INCS) -W -Wall -Wextra -O2 -std=c99
# Compiler flags for auto dependency generation:
CFLAGS  += -MMD -MP

LD      := $(CC)
LIBS    :=
LDFLAGS := $(LIBS)

STRIP   := strip
RM      := rm -f

BIN     := $(PROJECT)
SRC		:= $(wildcard *.c)
OBJ     := $(SRC:%.c=%.o)
DEP		:= $(OBJ:%.o=%.d)
SELF    := $(lastword $(MAKEFILE_LIST))


.PHONY: all clean distclean

all: $(BIN)

$(BIN): $(OBJ) $(SELF)
	$(LD) $(LDFLAGS) $(OBJ) -o $(BIN)
	$(STRIP) $(BIN)

$(OBJ): config.h 

%.o: %.c $(SELF) config.h 
	$(CC) -c $(CFLAGS) -o $*.o $*.c 
	
config.h: config.def.h
	cp $< $@

clean:
	$(RM) $(BIN) $(OBJ) $(DEP)
	
distclean: clean
	$(RM) config.h

-include $(DEP)

###########
##  EOF  ##
###########
