#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

enum Screen {
	SCREEN_START,
};

struct Controller {
	char* notification;	
	enum Screen screen;
};

void controllerStart();

#endif
