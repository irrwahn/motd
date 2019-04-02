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

GOBLD   := go build

STRIP   := strip
RM      := rm -f

BIN     := $(PROJECT)
SRC     := $(wildcard *.c)
OBJ     := $(SRC:%.c=%.o)
DEP     := $(OBJ:%.o=%.d)
SRV     := gotd
SRVSRC  := $(SRV).go
SELF    := $(lastword $(MAKEFILE_LIST))


.PHONY: all server clean distclean

all: $(BIN)

$(BIN): $(OBJ) $(SELF)
	$(LD) $(LDFLAGS) $(OBJ) -o $(BIN)
	$(STRIP) $(BIN)

$(OBJ): config.h

%.o: %.c $(SELF) config.h
	$(CC) -c $(CFLAGS) -o $*.o $*.c

config.h: config.def.h
	cp $< $@

server: $(SRVSRC)
	$(GOBLD) $(SRVSRC)

clean:
	$(RM) $(BIN) $(OBJ) $(DEP) $(SRV)

distclean: clean
	$(RM) config.h

-include $(DEP)

###########
##  EOF  ##
###########
