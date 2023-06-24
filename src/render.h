#ifndef RENDER_H
#define RENDER_H


#include "window_manager.h"
#include "menu.h"
#include "vector.h"
#include "game_menu.h"
#include "ui.h"


void render_init(struct WindowManager* window_manager);
void render(struct Vector center, struct UI* ui);


#endif
