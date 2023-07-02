#include "game_menu.h"


const char* g_menu_items[] = {
  "Resume",
  "New Game",
  "Manual",
  "Quit"
};


void game_menu_init(struct GameMenu* game_menu) {
  game_menu->selected = 0;
}


void game_menu_move_cursor_up(struct GameMenu* game_menu) {
  log_info("Move cursor up.");
  int length = array_size(g_menu_items);
  int selected = game_menu->selected - 1;
  if (selected < 0) selected = length - 1;
  log_info_f("selected=%s", g_menu_items[selected]);
  game_menu->selected = selected;
}


void game_menu_move_cursor_down(struct GameMenu* game_menu) {
  log_info("Move cursor down.");
  int length = array_size(g_menu_items);
  game_menu->selected = (game_menu->selected + 1) % length;
  log_info_f("selected=%s", g_menu_items[game_menu->selected]);
}


enum GameMenuCommand game_menu_get_selected(struct GameMenu* game_menu) {
  return game_menu->selected;
}

