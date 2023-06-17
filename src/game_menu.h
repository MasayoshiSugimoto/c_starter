#ifndef GAME_MENU_H
#define GAME_MENU_H


#include "util.h"
#include "window_manager.h"
#include "log.h"
#include "game.h"


struct GameMenu {
  int selected;
  bool enabled;
} g_game_menu;


void game_menu_init();
void game_menu_move_cursor_up();
void game_menu_move_cursor_down();
bool game_menu_is_enabled();
void game_menu_enable();
enum GameState game_menu_validate();


#endif
