#ifndef MENU_H
#define MENU_H

#include <curses.h>
#include <ncurses.h>
#include "log.h"
#include "game_window.h"


#define MENU_WIDTH 30
#define MENU_HEIGHT 14


enum MenuSelection {
  MENU_SELECTION_EASY = 0,
  MENU_SELECTION_MEDIUM = 1,
  MENU_SELECTION_HARD = 2,
  MENU_SELECTION_MAX = 3
};


enum MenuCommand {
  MENU_COMMAND_DO_NOTHING,
  MENU_COMMAND_SELECT_GAME_EASY,
  MENU_COMMAND_SELECT_GAME_MEDIUM,
  MENU_COMMAND_SELECT_GAME_HARD
};


const char* MENU_SELECTION_NAMES[] = {
  "easy",
  "medium",
  "hard"
};


struct Menu {
  enum MenuSelection menu_selection;
};


const char* menu_selection_as_string(enum MenuSelection selection) {
  if (selection < MENU_SELECTION_MAX) {
    return MENU_SELECTION_NAMES[selection];
  } else {
    log_fatal_f("Invalid menu selection: %d", selection);
  }
}


enum MenuCommand menu_command_from_selection(enum MenuSelection selection) {
  return MENU_COMMAND_SELECT_GAME_EASY + selection;
}


struct GameWindow* menu_get_window() {
  return &g_game_windows[GAME_WINDOW_ID_MENU];
}


void menu_init(struct Menu* menu) {
  menu->menu_selection = MENU_SELECTION_MEDIUM;
  struct GameWindow* game_window = menu_get_window();
  game_window->width = MENU_WIDTH;
  game_window->height = MENU_HEIGHT;
}


void menu_render(struct Menu* menu) {
  struct GameWindow* game_window = menu_get_window();

  WINDOW* window = game_window->window;

  int text_x = game_window->width / 2 - 11;
  int text_y = 3;
  mvwaddstr(window, text_y, text_x, "CHOOSE YOUR DIFFICULTY");
  mvwaddstr(window, text_y + 1, text_x, "======================");

  int start_x = 10;
  int start_y = 6;
  mvwaddstr(window, start_y + 0, start_x + 2, "Easy");
  mvwaddstr(window, start_y + 2, start_x + 2, "Medium");
  mvwaddstr(window, start_y + 4, start_x + 2, "Hard");

  // Render cursor.
  mvwaddch(window, start_y + (menu->menu_selection * 2), start_x, '>');
}



void menu_move_cursor_up(struct Menu* menu) {
  log_info("Up key pressed.");
  if (menu->menu_selection == 0) {
    menu->menu_selection = MENU_SELECTION_HARD;
  } else {
    menu->menu_selection = menu->menu_selection - 1;
  }
  log_info_f(
      "menu->menu_selection=%s",
      menu_selection_as_string(menu->menu_selection)
  );
}


void menu_move_cursor_down(struct Menu* menu) {
  log_info("Down key pressed.");
  menu->menu_selection = (menu->menu_selection + 1) % MENU_SELECTION_MAX;
  log_info_f(
      "menu->menu_selection=%s",
      menu_selection_as_string(menu->menu_selection)
  );
}


#endif
