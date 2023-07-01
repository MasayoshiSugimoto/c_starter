#ifndef GAME_MENU_H
#define GAME_MENU_H


#include "util.h"
#include "window_manager.h"
#include "log.h"
#include "game.h"


enum GameMenuCommand {
  GAME_MENU_RESUME,
  GAME_MENU_NEW_GAME,
  GAME_MENU_MANUAL,
  GAME_MENU_QUIT,
  GAME_MENU_COMMAND_MAX
};


struct GameMenu {
  enum GameMenuCommand selected;
  bool enabled;
};


void game_menu_init(struct GameMenu* game_menu);
void game_menu_move_cursor_up(struct GameMenu* game_menu);
void game_menu_move_cursor_down(struct GameMenu* game_menu);
bool game_menu_is_enabled(struct GameMenu* game_menu);
void game_menu_enable(struct GameMenu* game_menu);


#endif
