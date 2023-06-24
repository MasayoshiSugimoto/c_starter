#ifndef GAME_MENU_H
#define GAME_MENU_H


#include "util.h"
#include "window_manager.h"
#include "log.h"
#include "game.h"


struct GameMenu {
  int selected;
  bool enabled;
};


void game_menu_init(struct GameMenu* game_menu);
void game_menu_move_cursor_up(struct GameMenu* game_menu);
void game_menu_move_cursor_down(struct GameMenu* game_menu);
bool game_menu_is_enabled(struct GameMenu* game_menu);
void game_menu_enable(struct GameMenu* game_menu);
enum GameState game_menu_validate(struct GameMenu* game_menu);


#endif
