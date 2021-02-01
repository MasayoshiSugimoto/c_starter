#include "ui.h"
#include <stdio.h>

/********************************************************************************
* UI
********************************************************************************/


void uiPlayScreen(const char* notification, const char* board) {
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
		board,
		notification
	);
}
