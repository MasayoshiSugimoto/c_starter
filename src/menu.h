#ifndef MENU_H
#define MENU_H

#include <curses.h>
#include <ncurses.h>
#include "log.h"


#define MENU_WIDTH 30
#define MENU_HEIGHT 14


enum MenuSelection {
  MENU_SELECTION_EASY = 0,
  MENU_SELECTION_MEDIUM = 1,
  MENU_SELECTION_HARD = 2,
  MENU_SELECTION_MAX = 3
};


const char* MENU_SELECTION_NAMES[] = {
  "easy",
  "medium",
  "hard"
};


struct Menu {
  WINDOW* window;
  enum MenuSelection menu_selection;
};


int g_menu_warning_suppressor;  // This variable is just there to avoid warnings.


const char* menu_selection_as_string(enum MenuSelection selection) {
  if (selection < MENU_SELECTION_MAX) {
    return MENU_SELECTION_NAMES[selection];
  } else {
    log_fatal_f("Invalid menu selection: %d", selection);
  }
}


void menu_init(struct Menu* menu) {
	int width = MENU_WIDTH;
  int height = MENU_HEIGHT;
	int starty = 0;
	int startx = 0;

	menu->window = newwin(height, width, starty, startx);
  menu->menu_selection = MENU_SELECTION_MEDIUM;
}


int menu_get_width(struct Menu* menu) {
  int width;
  getmaxyx(menu->window, g_menu_warning_suppressor, width);
  return width;
}


int menu_get_height(struct Menu* menu) {
  int height;
  getmaxyx(menu->window, height, g_menu_warning_suppressor);
  return height;
}


void menu_set_position(struct Menu* menu, int x, int y) {
  mvwin(menu->window, y, x);
}


void menu_render(struct Menu* menu) {
  WINDOW* window = menu->window;
  box(window, 0, 0);

  int text_x = menu_get_width(menu) / 2 - 11;
  int text_y = 3;
  mvwaddstr(window, text_y, text_x, "CHOOSE YOUR DIFFICULTY");
  mvwaddstr(window, text_y + 1, text_x, "======================");

  int start_x = 10;
  int start_y = 6;
  mvwaddstr(window, start_y + 0, start_x + 2, "Easy");
  mvwaddstr(window, start_y + 2, start_x + 2, "Medium");
  mvwaddstr(window, start_y + 4, start_x + 2, "Hard");

  mvwaddch(window, start_y + (menu->menu_selection * 2), start_x, '>');

	wrefresh(window);
}


void menu_erase(struct Menu* menu) {
  werase(menu->window);
}


void menu_update_input(struct Menu* menu, int input) {
  switch (input) {
    case KEY_DOWN:
      log_info("Down key pressed.");
      menu->menu_selection = (menu->menu_selection + 1) % MENU_SELECTION_MAX;
      log_info_f(
          "menu->menu_selection=%s",
          menu_selection_as_string(menu->menu_selection)
      );
      break;
    case KEY_UP:
      log_info("Up key pressed.");
      int selection = menu->menu_selection - 1;
      if (selection < 0) {
        selection = MENU_SELECTION_HARD;
      }
      menu->menu_selection = selection;
      log_info_f(
          "menu->menu_selection=%s",
          menu_selection_as_string(menu->menu_selection)
      );
      break;
  }
}


#endif
