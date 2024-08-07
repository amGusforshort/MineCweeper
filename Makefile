
CC = cc
OPTIONS = -Wall -Wextra -pedantic

all: minecweeper

minecweeper: main.c
	$(CC) $(OPTIONS) -o $@ $<
