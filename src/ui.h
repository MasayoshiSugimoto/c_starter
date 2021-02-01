#ifndef _UI_H_
#define _UI_H_

#include "game.h"

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

void uiPlayScreen(struct UI* ui, const char* board);
void uiInit(struct UI* ui);

#endif
