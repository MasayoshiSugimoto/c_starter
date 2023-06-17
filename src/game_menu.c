#include "game_menu.h"


enum GameMenuCommand {
  GAME_MENU_RESUME,
  GAME_MENU_NEW_GAME,
  GAME_MENU_MANUAL,
  GAME_MENU_QUIT,
  GAME_MENU_COMMAND_MAX
};


const char* g_menu_items[] = {
  "Resume",
  "New Game",
  "Manual",
  "Quit"
};


void game_menu_init() {
  g_game_menu.selected = 0;
  g_game_menu.enabled = true;
}


void game_menu_move_cursor_up() {
  log_info("Move cursor up.");
  int length = array_size(g_menu_items);
  int selected = g_game_menu.selected - 1;
  if (selected < 0) selected = length - 1;
  log_info_f("selected=%s", g_menu_items[selected]);
  g_game_menu.selected = selected;
}


void game_menu_move_cursor_down() {
  log_info("Move cursor down.");
  int length = array_size(g_menu_items);
  g_game_menu.selected = (g_game_menu.selected + 1) % length;
  log_info_f("selected=%s", g_menu_items[g_game_menu.selected]);
}


enum GameMenuCommand game_menu_get_selected() {
  return g_game_menu.selected;
}


bool game_menu_is_enabled() {
  return g_game_menu.enabled;
}


void game_menu_enable() {
  g_game_menu.enabled = true;
}


enum GameState game_menu_validate() {
  switch (g_game_menu.selected) {
    case GAME_MENU_RESUME:
      log_info("Resuming game.");
      g_game_menu.enabled = false;
      return GAME_STATE_MAX;
    case GAME_MENU_NEW_GAME:
      log_info("Starting new game.");
      g_game_menu.enabled = false;
      return GAME_STATE_MENU;
    case GAME_MENU_MANUAL:
      log_info("Opening manual.");
      g_game_menu.enabled = false;
      return GAME_STATE_MAX;
    case GAME_MENU_QUIT:
      log_info("Quiting...");
      return GAME_STATE_QUIT;
    default:
      log_fatal_f("Invalid menu selection: %d", g_game_menu.selected);
  }
  return GAME_STATE_MAX;
}
