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

/********************************************************************************
* Main
********************************************************************************/

#define DEFAULT_TEXT_BUF 512

#define DEBUG_GAME_BOARD_SHOW_ALL false
#define DEBUG_ENABLE_TEST true

#define TERMINAL_MIN_HEIGHT 16


#if DEBUG_ENABLE_TEST

void debug_init() {
}

#endif



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

  game_init_easy_mode(&game);

  if (DEBUG_GAME_BOARD_SHOW_ALL) game_board_show_all(game_board);

  enum GameState game_state = GAME_STATE_MENU;

  struct Inputs inputs;
  input_init(&inputs);

  struct Terminal terminal;

  struct Menu menu;
  menu_init(&menu);

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
      menu_reset_window(&menu);
      continue;
    }


    {  // Render.
      erase();
      switch (game_state) {
        case GAME_STATE_IN_GAME:
        case GAME_STATE_GAME_OVER:
        case GAME_STATE_GAME_WON:
          game_render_in_game(&game, game_state, center);
          refresh();
          break;
        case GAME_STATE_MENU:
          refresh();
          render_menu_state(&terminal, &menu);
          break;
        default:
          log_fatal_f("Invalid game_state: %d", game_state);
      }
    }

    // Update inputs.
    int input = getch();
    enum MenuCommand menu_command = MENU_COMMAND_DO_NOTHING;
    switch (game_state) {
      case GAME_STATE_IN_GAME:
        input_update_in_game(&inputs, &game, input);
        break;
      case GAME_STATE_GAME_OVER:
        input_update_game_over(&inputs, &game, input);
        game_state = GAME_STATE_MENU;
        continue;
      case GAME_STATE_GAME_WON:
        input_update_game_won(&inputs, &game, input);
        game_state = GAME_STATE_MENU;
        continue;
      case GAME_STATE_MENU:
        menu_command = menu_update_input(&menu, input);
        break;
      default:
        log_fatal_f("Invalid game_state=%d", game_state);
    }
    if (inputs.is_quit) break;

    // Update game state.
    switch (game_state) {
      case GAME_STATE_IN_GAME:
        if (game_board_is_lost(game_board)) {
          game_state = GAME_STATE_GAME_OVER;
        } else if (game_board_is_win(game_board)) {
          game_state = GAME_STATE_GAME_WON;
        }
        break;
      case GAME_STATE_GAME_OVER:
      case GAME_STATE_GAME_WON:
        break;
      case GAME_STATE_MENU:
        // State change based on events.
        switch (menu_command) {
          case MENU_COMMAND_SELECT_GAME_EASY:
            game_init_easy_mode(&game);
            game_state = GAME_STATE_IN_GAME;
            break;
          case MENU_COMMAND_SELECT_GAME_MEDIUM:
            game_init_medium_mode(&game);
            game_state = GAME_STATE_IN_GAME;
            break;
          case MENU_COMMAND_SELECT_GAME_HARD:
            game_init_hard_mode(&game);
            game_state = GAME_STATE_IN_GAME;
            break;
          case MENU_COMMAND_DO_NOTHING:
            break;
        }
        break;
      default:
        log_fatal_f("Invalid game_state: %d", game_state);
    }

  }

  endwin();  // End ncurses.
  return 0;
}

