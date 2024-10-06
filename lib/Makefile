# Makefile

CC = gcc
CFLAGS = --std=c99 -I../include -pedantic -Wall \
-Wshadow -Wpointer-arith -Wcast-qual \
-Wstrict-prototypes -Wmissing-prototypes \
-Wdeclaration-after-statement
OBJDIR := ../obj
SOURCES := $(wildcard ./*.c)
OBJS := $(addprefix $(OBJDIR)/, ${SOURCES:.c=.o})

all: ${OBJS}

$(OBJDIR)/%.o: %.c
	@ mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
