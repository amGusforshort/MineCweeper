
CC = cc
OPTIONS = -g -Wall -Wextra -pedantic

all: minecweeper

minecweeper: main.c
	$(CC) $(OPTIONS) -o $@ $<
