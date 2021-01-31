#include "def.h"
#include "ui.h"
#include <stdio.h>

/********************************************************************************
* UI
********************************************************************************/

void uiInit(struct UI* ui) {
	ui->notification = "";
	ui->screen = UI_SCREEN_START;
}

void uiPlayScreen(struct UI* ui, struct Game* game) {
	char buf[BUFF_SMALL_SIZE];
	gameBoardAsString(buf, game);
	printf(
		"TICTACTOE\n"
		"=========\n"
		"\n"
		"--------------------\n"
		"\n"
		"%s\n"
		"\n"
		"--------------------\n"
		"\n"
		"Notification: %s\n"
		"\n"
		"--------------------\n"
		"\n"
		"Type a letter and type <ENTER>:\n",
		buf,
		ui->notification
	);

	char c;
	int nbMatch = scanf(" %c", &c);
	if (nbMatch != 1) {
		ui->notification = "Invalid input.";
		return;
	}

	struct BoardCell cell = gameGetBoardCell(c);
	if (!boardCellValid(&cell) || gameIsEmptyCell(game, &cell)) {
		ui->notification = "Invalid input.";
		return;
	}

	gamePlay(game, &cell);

	if (gameIsOver(game)) {
		ui->notification = "GAME OVER";
		return;
	}
}

