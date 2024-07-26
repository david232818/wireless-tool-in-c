# Makefile

CC = gcc
CFLAGS = --std=c99 -Wall -I../include
OBJDIR := ../obj
SOURCES := $(wildcard ./*.c)
OBJS := $(addprefix $(OBJDIR)/, ${SOURCES:.c=.o})

all: ${OBJS}

$(OBJDIR)/%.o: %.c
	@ mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
