#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <curses.h>
#include <ncurses.h>
#include <term.h>
#include <unistd.h>
#include "menu.h"
#include "log.h"
#include "cursor.h"
#include "vector.h"
#include "game_board.h"
#include "util.h"
#include "terminal.h"
#include "game.h"
#include "input.h"
#include "game_window.h"
#include "game_menu.h"
#include "render.h"

/********************************************************************************
* Main
********************************************************************************/

#define DEFAULT_TEXT_BUF 512

#define DEBUG_GAME_BOARD_SHOW_ALL false
#define DEBUG_ENABLE_TEST false

#define TERMINAL_MIN_HEIGHT 20


#if DEBUG_ENABLE_TEST

void debug_init() {
  log_info("DEBUG MODE ON");
  log_info("=============\n");

  game_window_init();

  struct Terminal terminal;
  terminal_init(&terminal);

  while (true) {
    erase();

    game_window_enable_only(GAME_WINDOW_ID_MENU);
    game_window_prepare_render();
    struct GameWindow* game_window = &g_game_windows[GAME_WINDOW_ID_MENU];
    game_window->left = terminal.width / 2 - game_window->left / 2;
    game_window->top = terminal.height / 2 - game_window->top / 2;

    refresh();
    game_window_render();

//    curs_set(CURSOR_VISIBILITY_INVISIBLE);
//    move(0, 0);

    getch();  // Wait for resize.
  }

}

#endif


enum MenuCommand menu_update_input(struct Menu* menu, int input) {
  switch (input) {
    case KEY_DOWN:
      menu_move_cursor_down(menu);
      return MENU_COMMAND_DO_NOTHING;
    case KEY_UP:
      menu_move_cursor_up(menu);
      return MENU_COMMAND_DO_NOTHING;
    case KEY_RESIZE:
      return MENU_COMMAND_DO_NOTHING;
    default:
      return menu_command_from_selection(menu->menu_selection);
  }
}


enum GameMenuCommand main_menu_update_input(int input) {
  switch (input) {
    case KEY_DOWN:
      log_info("Down key pressed.");
      game_menu_move_cursor_down();
      return GAME_MENU_COMMAND_MAX;
    case KEY_UP:
      log_info("Up key pressed.");
      game_menu_move_cursor_up();
      return GAME_MENU_COMMAND_MAX;
    case KEY_RESIZE:
      log_info("Window resized.");
      return GAME_MENU_COMMAND_MAX;
    default:
      log_info_f("Key pressed: %d", input);
      return game_menu_get_selected();
  }
}


enum GameState input_update(
    struct Game* game,
    struct Inputs* inputs,
    struct Menu* menu
) {
  int input = getch();
  enum MenuCommand menu_command = MENU_COMMAND_DO_NOTHING;
  enum GameMenuCommand game_menu_command = GAME_MENU_COMMAND_MAX;
  enum GameState game_state = game->game_state;
  struct GameBoard* game_board = &game->game_board;

  if (game_menu_is_enabled()) {
    log_info("Game menu is enabled.");
    game_menu_command = main_menu_update_input(input);
    if (game_menu_command == GAME_MENU_QUIT) return GAME_STATE_QUIT;
    if (game_menu_command < GAME_MENU_COMMAND_MAX) {
      return game_menu_update_game_state(game_menu_command);
    }
  } else if (game_state == GAME_STATE_IN_GAME) {
    input_update_in_game(inputs, game, input);
    if (game_board_is_lost(game_board)) {
      return GAME_STATE_GAME_OVER;
    } else if (game_board_is_win(game_board)) {
      return GAME_STATE_GAME_WON;
    }
  } else if (game_state == GAME_STATE_GAME_OVER) {
    input_update_game_over(inputs, game, input);
    game_menu_enable();
  } else if (game_state == GAME_STATE_GAME_WON) {
    input_update_game_won(inputs, game, input);
    game_menu_enable();
  } else if (game_state == GAME_STATE_MENU) {
    menu_command = menu_update_input(menu, input);
    // State change based on events.
    switch (menu_command) {
      case MENU_COMMAND_SELECT_GAME_EASY:
        game_init_easy_mode(game);
        return GAME_STATE_IN_GAME;
      case MENU_COMMAND_SELECT_GAME_MEDIUM:
        game_init_medium_mode(game);
        return GAME_STATE_IN_GAME;
      case MENU_COMMAND_SELECT_GAME_HARD:
        game_init_hard_mode(game);
        return GAME_STATE_IN_GAME;
      case MENU_COMMAND_DO_NOTHING:
        break;
    }
  } else {
    log_fatal_f("Invalid game_state=%d", game_state);
  }

  return GAME_STATE_MAX;
}


int main() {
  log_init();

  // Initialize ncurses
  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  srand(time(NULL));

  struct Game game;
  struct GameBoard* game_board = &game.game_board;

  game_init_medium_mode(&game);

  if (DEBUG_GAME_BOARD_SHOW_ALL) game_board_show_all(game_board);

  struct Inputs inputs;
  input_init(&inputs);

  struct Terminal terminal;

  game_window_init();
  render_game_won_init();

  struct Menu menu;
  menu_init(&menu);
  render_game_over_init();
  game_menu_init();

#if DEBUG_ENABLE_TEST
  debug_init();
#endif

  // Loop to track cursor position
  while (true) {
    terminal_init(&terminal);
    log_info_f("terminal={width:%d, height:%d}", terminal.width, terminal.height);
    struct Vector center = terminal_center(&terminal);

    if (terminal.height < TERMINAL_MIN_HEIGHT) {
      log_info("Terminal height is less than the minimum allowed.");
      erase();
      addstr(
          "Terminal height is less than the minimum allowed.\n"
          "Please resize the terminal.\n"
      );
      refresh();
      getch();  // Wait for resize.
      continue;
    }

    render(&game, center, &menu);

    enum GameState game_state = input_update(&game, &inputs, &menu);
    if (game_state < GAME_STATE_MAX) {
      game.game_state = game_state;
    }

    if (game_state == GAME_STATE_QUIT) {
      break;
    }

    log_info_f("Game state: %s", g_game_state_strings[game.game_state]);
  }

  endwin();  // End ncurses.
  return 0;
}

