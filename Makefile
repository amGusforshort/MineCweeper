
CC = cc
OPTIONS = -Wall -Wextra -pedantic

all: minecweeper

run: minecweeper
	./minecweeper

minecweeper: main.c
	$(CC) $(OPTIONS) -o $@ $<
