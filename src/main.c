#include "game.h"
#include <stdio.h>

const int BUFF_SMALL_SIZE = 1024;

/********************************************************************************
* UI
********************************************************************************/

enum UIScreen {
	UI_SCREEN_START,
};

struct UI {
	char* notification;	
	enum UIScreen screen;
};

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

/********************************************************************************
* Main
********************************************************************************/

int main() {
	struct Game game;
	gameInit(&game);

	struct UI ui;
	uiInit(&ui);

	while (true) {
		switch (ui.screen) {
			case UI_SCREEN_START:
				uiPlayScreen(&ui, &game);
				break;
			default:
				uiPlayScreen(&ui, &game);
		}
	}

	return 0;
}
