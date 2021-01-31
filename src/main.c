#include "game.h"
#include "ui.h"


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
