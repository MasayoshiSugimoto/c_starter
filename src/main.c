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
#include "game_menu.h"
#include "render.h"
#include "window_manager.h"

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

  game_init_medium_mode(&game);

  if (DEBUG_GAME_BOARD_SHOW_ALL) game_board_show_all(game_board);

  struct Terminal terminal;

  window_manager_init();
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

    enum GameState game_state = input_update(&game, &menu);
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

