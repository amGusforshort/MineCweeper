
CC = cc
FLAGS = -Wall -Wextra -pedantic
OPTIONS = -O3

EXE = minecweeper
SRC = main

all: $(EXE)

$(EXE): $(SRC).c
	$(CC) $(OPTIONS) $(FLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -frv $(EXE)

