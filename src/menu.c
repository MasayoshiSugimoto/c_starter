#include "menu.h"


#define MENU_WIDTH 30
#define MENU_HEIGHT 14


const char* MENU_SELECTION_NAMES[] = {
  "easy",
  "medium",
  "hard"
};


const char* menu_selection_as_string(enum MenuSelection selection) {
  if (selection < MENU_SELECTION_MAX) {
    return MENU_SELECTION_NAMES[selection];
  } else {
    log_fatal_f("Invalid menu selection: %d", selection);
  }
}


void menu_init(struct Menu* menu) {
  menu->menu_selection = MENU_SELECTION_MEDIUM;
  window_manager_set_width(WINDOW_ID_MENU, MENU_WIDTH);
  window_manager_set_height(WINDOW_ID_MENU, MENU_HEIGHT);
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


bool menu_validate(struct Menu* menu, struct Game* game) {
  bool is_new_game = false;
  switch (menu->menu_selection) {
    case MENU_SELECTION_EASY:
      game_init_easy_mode(game);
      is_new_game = true;
      break;
    case MENU_SELECTION_MEDIUM:
      game_init_medium_mode(game);
      is_new_game = true;
      break;
    case MENU_SELECTION_HARD:
      game_init_hard_mode(game);
      is_new_game = true;
      break;
    default:
      log_fatal_f("Invalid menu selection: %d", menu->menu_selection);
  }
  return is_new_game;
}
