
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define IN_BUF_SIZE 256

#define CELLS_AT(board, x, y) (board)->cells[(y)*(board)->width + (x)]

typedef struct {
	bool is_revealed;
	bool is_flagged;
	bool is_mine;
	size_t mine_num;
} Cell;

typedef struct {
	size_t width;
	size_t height;
	size_t mine_num;
	Cell *cells;
} Board;

size_t revealed_cells = 0;

void init_board(Board *board) {
	board->cells = malloc(sizeof(*board->cells) * board->width * board->height);
	if (board->cells == NULL) {
		perror("ERROR: malloc");
		exit(EXIT_FAILURE);
	}
	for (size_t i = 0; i < board->mine_num; i++) {
		size_t x;
		size_t y;
		do {
			x = rand() % board->width;
			y = rand() % board->height;
		} while (CELLS_AT(board, x, y).is_mine);
		CELLS_AT(board, x, y).is_mine = true;
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
			if (nx >= board->width || ny >= board->height) continue;
			if (CELLS_AT(board, x, y).is_mine) continue;
			CELLS_AT(board, x, y).mine_num++;
		}
	}
	for (size_t y = 0; y < board->height; y++) {
		for (size_t x = 0; x < board->width; x++) {
			CELLS_AT(board, x, y).is_revealed = false;
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

void print_board(Board *board) {
	printf("   X ");
	for (size_t i = 0; i < board->width; i++) {
		size_t units = i % 10;
		printf("%zu%c", units, " |"[units == 9]);
	}
	printf("\n Y +");
	for (size_t i = 0; i < board->width*2 + 1; i++) {
		printf("-");
	}
	printf("+\n");

	for (size_t y = 0; y < board->height; y++) {
		for (size_t x = 0; x < board->width; x++) {
			if (x == 0) {
				printf("%2zu | ", y);
			}
			print_cell(&CELLS_AT(board, x, y));
		}
		printf("|\n");
	}
	printf("   +");
	for (size_t i = 0; i < board->width*2 + 1; i++) {
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

void reveal_cell(Board *board, size_t x, size_t y) {
	CELLS_AT(board, x, y).is_revealed = true;
	revealed_cells++;

	if (CELLS_AT(board, x, y).mine_num == 0) {
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
			if (nx >= board->width || ny >= board->height) continue;
			if (board->cells[ny*board->width + nx].is_revealed) continue;
			reveal_cell(board, nx, ny);
		}
	}
}

void print_clean_board(Board *board) {
	for (size_t y = 0; y < board->height; y++) {
		for (size_t x = 0; x < board->width; x++) {
			if (CELLS_AT(board, x, y).is_mine) printf("# ");
			else printf("%zu ", CELLS_AT(board, x, y).mine_num);
		}
		printf("\n");
	}
}

void reveal_board(Board *board) {
	for (size_t y = 0; y < board->height; y++) {
		for (size_t x = 0; x < board->width; x++) {
			reveal_cell(board, x, y);
		}
	}
}

void refresh_screen(Board *board) {
	printf("\x1b[H");
	printf("\x1b[2J");
	print_board(board);
}

void meta_command(Board *board, char *buf) {
	if (strcmp(buf + 1, "pcb") == 0) {
		print_clean_board(board);
	} else {
		refresh_screen(board);
		printf("Unknown meta command: '%s'.\n", buf);
	}
}

void print_menu() {
	printf("\x1b[H");
	printf("\x1b[2J");
	printf("MineCweeper\n"
	       "Difficulty options\n"
	       "(E)asy - 9x9 grid, 10 mines\n"
	       "(M)edium - 16x16 grid, 40 mines\n"
	       "(H)ard - 30x16 grid, 99 mines\n"
	       "Choose difficulty:\n\n"
	       "(Type 'quit' to quit)\n\x1b[2A");
}

void print_help_menu() {
	printf("Symbols:\n"
	       "1 - 8: number of surrounding mines\n"
	       "!: flagged cell\n"
	       "#: mine\n"
	       ".: unrevealed cell\n"
	       "\n"
	       "Commands:\n"
	       "r y x: reveal cell at position (x,y)\n"
	       "f y x: flag/unflag cell at position (x,y)\n"
	       "help: prints this list\n"
	       "quit: quits the game\n");
}

int main(void) {
	srand(time(NULL));

	Board board = {0};
	char buf[IN_BUF_SIZE] = {0};

	for (;;) {
		print_menu();
		read_command(buf);
		if (strcmp(buf, "quit") == 0) return 0;

		if (strcmp(buf, "E") == 0) {
			board.width = board.height = 9;
			board.mine_num = 10;
		} else if (strcmp(buf, "M") == 0) {
			board.width = board.height = 16;
			board.mine_num = 40;
		} else if (strcmp(buf, "H") == 0) {
			board.width = 30;
			board.height = 16;
			board.mine_num = 99;
		} else {
			continue;
		}
		break;
	}

	init_board(&board);

	refresh_screen(&board);
	for (;;) {
		read_command(buf);
		refresh_screen(&board);

		if (buf[0] == '.') {
			meta_command(&board, buf);
			continue;
		}
		if (strcmp(buf, "quit") == 0) break;
		if (strcmp(buf, "help") == 0) {
			print_help_menu();
			continue;
		}

		char action;
		size_t x, y;
		if (sscanf(buf, "%c %zu %zu", &action, &y, &x) == 3) {
			if (x >= board.width || y >= board.height) {
				// refresh_screen(&board);
				printf("Invalid coordinates (%d,%d).\n", (int)x, (int)y);
				continue;
			}
			if (CELLS_AT(&board, x, y).is_revealed) {
				// refresh_screen(&board);
				printf("Cell is already revealed.\n");
				continue;
			}
			if (action == 'f') {
				CELLS_AT(&board, x, y).is_flagged = !CELLS_AT(&board, x, y).is_flagged;
			} else if (action == 'r') {
				if (CELLS_AT(&board, x, y).is_flagged) {
					// refresh_screen(&board);
					printf("Cell is flagged.\n");
					continue;
				}
				reveal_cell(&board, x, y);
				if (CELLS_AT(&board, x, y).is_mine) {
					reveal_board(&board);
					refresh_screen(&board);
					printf("You hit a mine! You lose!\n");
					break;
				} else if (revealed_cells == board.width * board.height - board.mine_num) {
					refresh_screen(&board);
					printf("You won!\n");
					break;
				}
			}
			refresh_screen(&board);
		} else {
			// refresh_screen(&board);
			printf("Invalid command: '%s'.\n", buf);
		}
	}

	free(board.cells);
	return 0;
}

