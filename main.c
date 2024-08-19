
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define IN_BUF_SIZE 128

#define CELLS_AT(board, x, y) (board)->cells[(y)*(board)->width + (x)]

#define MINE_OFFSET   0x05
#define FLAG_OFFSET   0x06
#define REVEAL_OFFSET 0x07
#define MINE_MASK     0x0f

#define IS_MINE(cell)     (((cell) >> MINE_OFFSET)   & 0x1) 
#define IS_FLAGGED(cell)  (((cell) >> FLAG_OFFSET)   & 0x1)
#define IS_REVEALED(cell) (((cell) >> REVEAL_OFFSET) & 0x1)
#define MINE_NUM(cell)    ((cell) & MINE_MASK)

#define SET_MINE(cell, bit)     ((cell) = ((cell) & ~(0x1 << MINE_OFFSET))   | ((bit) << 0x5))
#define SET_FLAGGED(cell, bit)  ((cell) = ((cell) & ~(0x1 << FLAG_OFFSET))   | ((bit) << 0x6))
#define SET_REVEALED(cell, bit) ((cell) = ((cell) & ~(0x1 << REVEAL_OFFSET)) | ((bit) << 0x7))

// rfmu nnnn
typedef uint8_t Cell;

typedef struct {
	size_t width;
	size_t height;
	size_t mine_num;
	Cell *cells;
} Board;

size_t revealed_cells = 0;

void init_game(Board *board) {
	revealed_cells = 0;
	board->cells = realloc(board->cells, board->width * board->height * sizeof(*board->cells));
	if (board->cells == NULL) {
		perror("ERROR: malloc");
		exit(EXIT_FAILURE);
	}
	memset(board->cells, 0, board->width * board->height * sizeof(Cell));

	for (size_t i = 0; i < board->mine_num; i++) {
		size_t x;
		size_t y;
		do {
			x = rand() % board->width;
			y = rand() % board->height;
		} while (IS_MINE(CELLS_AT(board, x, y)));
		SET_MINE(CELLS_AT(board, x, y), true);
		const int dirs[][2] = {
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
			if (IS_MINE(CELLS_AT(board, nx, ny))) continue;
			CELLS_AT(board, nx, ny)++;
		}
	}
}

void print_cell(Cell cell) {
	printf("\x1b[1m");
	if (IS_REVEALED(cell)) {
		printf("\x1b[47m");
		if (IS_MINE(cell)) {
			printf("\x1b[31m#");
		} else if (MINE_NUM(cell) != 0) {
			uint8_t num = MINE_NUM(cell);
			switch (num) {
				case 1:
					printf("\x1b[94m");
					break;
				case 2:
					printf("\x1b[32m");
					break;
				case 3:
					printf("\x1b[91m");
					break;
				case 4:
					printf("\x1b[34m");
					break;
				case 5:
					printf("\x1b[31m");
					break;
				case 6:
					printf("\x1b[36m");
					break;
				case 7:
					printf("\x1b[30m");
					break;
				case 8:
					printf("\x1b[90m");
					break;
				default:
					assert(false && "Unreachable");
			}
			printf("%u", num);
		} else {
			printf(" ");
		}
	} else {
		printf("\x1b[100m");
		if (IS_FLAGGED(cell)) {
			printf("\x1b[91m!");
		} else {
			printf("\x1b[37m.");
		}
		// printf("%c ", ".!"[IS_FLAGGED(cell)]);
	}
	printf(" \x1b[0m");
}

void print_board(Board *board) {
	printf("   X ");
	for (size_t i = 0; i < board->width; i++) {
		size_t units = i % 10;
		printf("%zu%c", units, " |"[units == 9]);
	}
	printf("\n Y\x1b[1;40;37m +");
	for (size_t i = 0; i < board->width*2 + 1; i++) {
		printf("-");
	}
	printf("+ \x1b[0m\n");

	for (size_t y = 0; y < board->height; y++) {
		printf("%2zu\x1b[1;40;37m |\x1b[%dm ", y, IS_REVEALED(CELLS_AT(board, 0, y)) ? 47 : 100);
		for (size_t x = 0; x < board->width; x++) {
			print_cell(CELLS_AT(board, x, y));
		}
		printf("\x1b[1;40;37m| \x1b[0m\n");
	}

	printf("  \x1b[1;40;37m +");
	for (size_t i = 0; i < board->width*2 + 1; i++) {
		printf("-");
	}
	printf("+ \n\x1b[0m");
}

void read_command(char *buf) {
	if (fgets(buf, IN_BUF_SIZE, stdin) == NULL) {
		perror("ERROR: fgets");
		exit(EXIT_FAILURE);
	}
	buf[strlen(buf) - 1] = '\0';
}

void reveal_cell(Board *board, size_t x, size_t y) {
	SET_REVEALED(CELLS_AT(board, x, y), true);
	revealed_cells++;

	if (MINE_NUM(CELLS_AT(board, x, y)) == 0) {
		const int dirs[][2] = {
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
			if (IS_REVEALED(CELLS_AT(board, nx, ny))) continue;
			reveal_cell(board, nx, ny);
		}
	}
}

void print_clean_board(Board *board) {
	for (size_t y = 0; y < board->height; y++) {
		for (size_t x = 0; x < board->width; x++) {
			if (IS_MINE(CELLS_AT(board, x, y))) printf("# ");
			else printf("%u ", MINE_NUM(CELLS_AT(board, x, y)));
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
	if (strcmp(buf, "pcb") == 0) {
		print_clean_board(board);
	} else {
		printf("Unknown meta command: '%s'.\n", buf);
	}
}

void print_menu(void) {
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

void print_help_menu(void) {
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

_begin_game:
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

	init_game(&board);

	refresh_screen(&board);
	for (;;) {
		read_command(buf);
		refresh_screen(&board);

		if (buf[0] == '.') {
			meta_command(&board, buf + 1);
			continue;
		}
		if (strcmp(buf, "quit") == 0) goto _exit_game;
		if (strcmp(buf, "help") == 0) {
			print_help_menu();
			continue;
		}

		char action;
		size_t x, y;
		if (sscanf(buf, "%c %zu %zu", &action, &y, &x) == 3) {
			if (x >= board.width || y >= board.height) {
				printf("Invalid coordinates (%d,%d).\n", (int)x, (int)y);
				continue;
			}
			if (IS_REVEALED(CELLS_AT(&board, x, y))) {
				printf("Cell is already revealed.\n");
				continue;
			}
			if (action == 'f') {
				SET_FLAGGED(CELLS_AT(&board, x, y), !IS_FLAGGED(CELLS_AT(&board, x, y)));
			} else if (action == 'r') {
				if (IS_FLAGGED(CELLS_AT(&board, x, y))) {
					printf("Cell is flagged.\n");
					continue;
				}
				reveal_cell(&board, x, y);
				if (IS_MINE(CELLS_AT(&board, x, y))) {
					reveal_board(&board);
					refresh_screen(&board);
					printf("You hit a mine! You lose!\n");
					break;
				} else if (revealed_cells == board.width*board.height - board.mine_num) {
					refresh_screen(&board);
					printf("You won!\n");
					break;
				}
			}
			refresh_screen(&board);
		} else {
			printf("Invalid command: '%s'.\n", buf);
		}
	}

	for (;;) {
		printf("Try again?\n\n(Y/N)\x1b[1F");
		read_command(buf);
		if (strcmp(buf, "Y") == 0) {
			goto _begin_game;
		} else if (strcmp(buf, "N") == 0) {
			break;
		}
	}

_exit_game:
	free(board.cells);
	return 0;
}

