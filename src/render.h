#ifndef RENDER_H
#define RENDER_H


#include "window_manager.h"
#include "menu.h"
#include "vector.h"
#include "game_menu.h"


void render_init();
void render(struct Game* game, struct Vector center, struct Menu* menu);


#endif
