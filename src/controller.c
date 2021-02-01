#include "controller.h"
#include "def.h"
#include "game.h"
#include "ui.h"
#include <stdio.h>

void init(struct Controller* ui) {
	ui->notification = "";
	ui->screen = SCREEN_START;
}

void playScreen(struct Controller* ui, struct Game* game) {
	char buf[BUFF_SMALL_SIZE];
	gameBoardAsString(buf, game);
	uiPlayScreen(ui, buf);

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

void controllerStart() {
	struct Game game;
	gameInit(&game);

	struct Controller controller;
	init(&controller);

	while (true) {
		switch (controller.screen) {
			case SCREEN_START:
				playScreen(&controller, &game);
				break;
			default:
				playScreen(&controller, &game);
		}
	}
}

