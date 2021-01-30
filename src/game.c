#include "game.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/********************************************************************************
* Game
********************************************************************************/

const char BOARD_CELLS[3][3] = {
	{ 'A', 'B', 'C' },
	{ 'D', 'E', 'F' },
	{ 'G', 'H', 'I' },
};

void boardCellInit(struct BoardCell* cell) {
	cell->x = -1;
	cell->y = -1;
}

bool boardCellValid(struct BoardCell* cell) {
	return 0 <= cell->x && cell->x < 3
		&& 0 <= cell->y && cell->y < 3;
}

void boardInit(Board* board) {
	memset(board, 0, sizeof board[0]);
}

void gameInit(struct Game* game) {
	boardInit(&game->board);
	game->turn = PLAYER_1;
}

char gameGetPlayerMark(enum Player player) {
	if (player == PLAYER_1) {
		return 'X';
	} else {
		return 'Y';
	}
}

char gameGetChar(struct Game* game, int x, int y) {
	enum Player p = game->board[x][y];
	if (p == PLAYER_1 || p == PLAYER_2) {
		return gameGetPlayerMark(p);
	} else {
		return BOARD_CELLS[y][x];
	}
}

void gameBoardAsString(char* buf, struct Game* game) {
	sprintf(
		buf,
		" %c | %c | %c \n"
		"---+---+---\n"
		" %c | %c | %c \n"
		"---+---+---\n"
		" %c | %c | %c ", 
		gameGetChar(game, 0, 0), gameGetChar(game, 1, 0), gameGetChar(game, 2, 0),
		gameGetChar(game, 0, 1), gameGetChar(game, 1, 1), gameGetChar(game, 2, 1),
		gameGetChar(game, 0, 2), gameGetChar(game, 1, 2), gameGetChar(game, 2, 2)
	);
}

bool gameIsEmptyCell(struct Game* game, struct BoardCell* cell) {
	return game->board[cell->x][cell->y] != PLAYER_NONE;
}

struct BoardCell gameGetBoardCell(char c) {
	struct BoardCell cell;
	boardCellInit(&cell);
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			if (BOARD_CELLS[y][x] == c) {
				cell.x = x;
				cell.y = y;
				return cell;
			}
		}
	}
	return cell;
}

void gamePlay(struct Game* game, struct BoardCell* cell) {
	game->board[cell->x][cell->y] = game->turn;
	if (game->turn == PLAYER_1) {
		game->turn = PLAYER_2;
	} else {
		game->turn = PLAYER_1;
	}
}

bool gameHasWon(struct Game* game, enum Player player) {
	static struct BoardCell winningPattern[8][3] = {
		// Vertical
		{{0, 0}, {0, 1}, {0, 2}},
		{{1, 0}, {1, 1}, {1, 2}},
		{{2, 0}, {2, 1}, {2, 2}},
		// Horizontal
		{{0, 0}, {1, 0}, {2, 0}},
		{{0, 1}, {1, 1}, {2, 1}},
		{{0, 2}, {1, 2}, {2, 2}},
		// Diagonal
		{{0, 0}, {1, 1}, {2, 2}},
		{{0, 2}, {1, 1}, {2, 0}},
	};

	for (int i = 0; i < 8; i++) {
		bool aligned = true;
		for (int j = 0; j < 3; j++) {
			struct BoardCell cell = winningPattern[i][j];
			if (game->board[cell.x][cell.y] != player) {
				aligned = false;
				break;
			}
		}
		if (aligned) {
			return true;
		}
	}

	return false;
}

bool gameIsOver(struct Game* game) {
	if (gameHasWon(game, PLAYER_1) || gameHasWon(game, PLAYER_2)) {
		return true;
	}

	// Check if there is a remaining cell.
	for (int x = 0; x < 3; x++)	{
		for (int y = 0; y < 3; y++) {
			if (game->board[x][y] == PLAYER_NONE) return false;
		}
	}
	return true;
}

