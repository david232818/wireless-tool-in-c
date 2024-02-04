# Makefile

CC = gcc
LIBS = -lc
CFLAGS = -std=c11 -Wall

SRC=$(wildcard *.c)

wltools: $(SRC)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
