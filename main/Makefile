# Makefile

CC = gcc
CFLAGS = --std=c99 -I../include -pedantic -Wall \
-Wshadow -Wpointer-arith -Wcast-qual \
-Wstrict-prototypes -Wmissing-prototypes \
-Wdeclaration-after-statement
EXENAME = wltool
BINDIR = ../bin
SOURCES := main.c
OBJDIR = ../obj
OBJS := $(addprefix $(OBJDIR)/, ${SOURCES:.c=.o})

all: ${OBJS}
	@ mkdir -p $(BINDIR)
	$(CC) $(OBJDIR)/*.o -o $(BINDIR)/$(EXENAME)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
