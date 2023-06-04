#ifndef GAME_MENU_H
#define GAME_MENU_H


#include "util.h"


#define GAME_MENU_WIDTH 31
#define GAME_MENU_HEIGHT 15


struct GameMenu {
  int selected;
  bool enabled;
} g_game_menu;


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


struct GameWindow* game_menu_get_window() {
  return &g_game_windows[GAME_WINDOW_ID_GAME_MENU];
}


void game_menu_init() {
  g_game_menu.selected = 0;
  g_game_menu.enabled = true;
  struct GameWindow* game_window = game_menu_get_window();
  game_window->width = GAME_MENU_WIDTH;
  game_window->height = GAME_MENU_HEIGHT;
}


void game_menu_render(int center_x, int center_y) {
  struct GameWindow* game_window = &g_game_windows[GAME_WINDOW_ID_GAME_MENU];
  int left = center_x - game_window->width / 2;
  int top = center_y - game_window->height / 2;
  WINDOW* window = game_window->window;
  mvwin(window, top, left);
  wresize(window, game_window->height, game_window->width);
  box(window, 0, 0);

  char* title = "MENU";
  int text_x = (game_window->width - strlen(title)) / 2;
  int text_y = 2;
  mvwaddstr(window, text_y, text_x, title);
  mvwaddstr(window, text_y + 1, text_x, "====");

  text_x = 12;
  text_y += 3;
  int space_y = 2;
  mvwaddstr(window, text_y, text_x, "Resume");
  mvwaddstr(window, text_y + space_y, text_x, "New Game");
  mvwaddstr(window, text_y + space_y * 2, text_x, "Manual");
  mvwaddstr(window, text_y + space_y * 3, text_x, "Quit");

  // Render cursor.
  mvwaddch(window, text_y + (g_game_menu.selected * space_y), 9, '>');
}


enum GameMenuCommand game_menu_update_input(int input) {
  int selected = g_game_menu.selected;
  int length = array_size(g_menu_items);
  switch (input) {
    case KEY_DOWN:
      log_info("Down key pressed.");
      selected = (selected + 1) % length;
      log_info_f("selected=%s", g_menu_items[selected]);
      g_game_menu.selected = selected;
      return GAME_MENU_COMMAND_MAX;
    case KEY_UP:
      log_info("Up key pressed.");
      selected--;
      if (selected < 0) selected = length - 1;
      log_info_f("selected=%s", g_menu_items[selected]);
      g_game_menu.selected = selected;
      return GAME_MENU_COMMAND_MAX;
    case KEY_RESIZE:
      log_info("Window resized.");
      return GAME_MENU_COMMAND_MAX;
    default:
      log_info_f("`%s` validated.", g_menu_items[selected]);
      return g_game_menu.selected;
  }
}


enum GameState game_menu_update_game_state(enum GameMenuCommand command) {
  if (command == GAME_MENU_COMMAND_MAX) return -1;

  switch (command) {
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
      log_fatal("Dead path reached.");
    default:
      log_fatal_f("Invalid command: %d", command);
  }
}


bool game_menu_is_enabled() {
  return g_game_menu.enabled;
}


void game_menu_enable() {
  g_game_menu.enabled = true;
}


#endif