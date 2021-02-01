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
}

