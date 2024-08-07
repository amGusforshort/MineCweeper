
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BOARD_WIDTH 9
#define BOARD_HEIGHT BOARD_WIDTH 
#define MINE_NUM 10

#define IN_BUF_SIZE 256

//   X >
//  +-----------------+
// Y|0 0 0 0 0 0 0 0 0|
// v|0 0 0 0 0 0 0 0 0|
//  |0 0 0 0 0 0 0 0 0|
//  |0 0 0 0 0 0 0 0 0
//  |0 0 0 0 0 0 0 0 0
//  |0 0 0 0 0 0 0 0 0
//  |0 0 0 0 0 0 0 0 0
//  |0 0 0 0 0 0 0 0 0
//  |0 0 0 0 0 0 0 0 0
//  +-----------------+

typedef struct {
	bool is_revealed;
	bool is_flagged;
	bool is_mine;
	size_t mine_num;
} Cell;

Cell board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

void init_board() {
	for (size_t i = 0; i < MINE_NUM; i++) {
		size_t x;
		size_t y;
		do {
			x = rand() % BOARD_WIDTH;
			y = rand() % BOARD_HEIGHT;
		} while (board[y][x].is_mine);
		board[y][x].is_mine = true;
		int dirs[][2] = {
			{-1, -1},
			{-1,  1},
			{ 1, -1},
			{ 1,  1},
			{ 0, -1},
			{ 0,  1},
			{-1,  0},
			{ 1,  0},
		};
		for (size_t j = 0; j < 8; j++) {
			size_t nx = x + dirs[j][0];
			size_t ny = y + dirs[j][1];
			if (nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT) continue;
			if (board[ny][nx].is_mine) continue;
			board[ny][nx].mine_num++;
		}
	}
	for (size_t y = 0; y < BOARD_HEIGHT; y++) {
		for (size_t x = 0; x < BOARD_WIDTH; x++) {
			board[y][x].is_revealed = false;
		}
	}
}

void print_cell(Cell *cell) {
	if (cell->is_revealed) {
		if (cell->is_mine) {
			printf("# ");
		} else if (cell->mine_num != 0) {
			printf("%zu ", cell->mine_num);
		} else {
			printf("  ");
		}
	} else {
		printf("%c ", ".!"[cell->is_flagged]);
	}
}

void print_board() {
	printf("+");
	for (int i = 0; i < BOARD_WIDTH*2 + 1; i++) {
		printf("-");
	}
	printf("+\n");

	for (size_t y = 0; y < BOARD_HEIGHT; y++) {
		for (size_t x = 0; x < BOARD_WIDTH; x++) {
			if (x == 0) {
				printf("| ");
			}
			print_cell(&board[y][x]);
		}
		printf("|\n");
	}

	printf("+");
	for (int i = 0; i < BOARD_WIDTH*2 + 1; i++) {
		printf("-");
	}
	printf("+\n");
}

void read_command(char *buf) {
	if (fgets(buf, IN_BUF_SIZE, stdin) == NULL) {
		perror("ERROR: fgets");
		exit(EXIT_FAILURE);
	}
	buf[strlen(buf) - 1] = '\0';
}

void reveal_cell(size_t x, size_t y) {
	board[y][x].is_revealed = true;

	if (board[y][x].mine_num == 0) {
		int dirs[][2] = {
			{-1, -1},
			{-1,  1},
			{ 1, -1},
			{ 1,  1},
			{ 0, -1},
			{ 0,  1},
			{-1,  0},
			{ 1,  0},
		};
		for (size_t j = 0; j < 8; j++) {
			size_t nx = x + dirs[j][0];
			size_t ny = y + dirs[j][1];
			if (nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT) continue;
			if (board[ny][nx].is_revealed) continue;
			reveal_cell(nx, ny);
		}
	}
}

int main(void) {
	srand(time(NULL));
	// srand(69);
	init_board();

	char buf[IN_BUF_SIZE] = {0};
	for (;;) {
		print_board();
		read_command(buf);
		if (strcmp(buf, "quit") == 0) break;
		char action;
		size_t x, y;
		if(sscanf(buf, "%c %zu %zu", &action, &y, &x) == 3) {
			if (x >= BOARD_WIDTH || y >= BOARD_HEIGHT) {
				printf("Invalid coordinates (%d,%d).\n", (int)x, (int)y);
				continue;
			}
			if (board[y][x].is_revealed) {
				printf("Cell is already revealed.\n");
				continue;
			}
			if (action == 'f') {
				board[y][x].is_flagged = !board[y][x].is_flagged;
			} else if (action == 'r') {
				if (board[y][x].is_flagged) {
					printf("Cell is flagged.\n");
				}
				reveal_cell(x, y);
			}
		} else {
			printf("Invalid command: '%s'.\n", buf);
		}
	}
	return 0;
}

