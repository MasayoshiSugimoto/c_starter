#include "ui.h"
#include "game.h"
#include "def.h"
#include <stdio.h>

void playScreen(struct UI* ui, struct Game* game) {
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

	struct UI ui;
	uiInit(&ui);

	while (true) {
		switch (ui.screen) {
			case UI_SCREEN_START:
				playScreen(&ui, &game);
				break;
			default:
				playScreen(&ui, &game);
		}
	}
}

