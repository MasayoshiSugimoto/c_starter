#include <time.h>
#include "input.h"
#include "render.h"
#include "ui.h"


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


struct UI ui;


int main() {
  log_init();

  // Initialize ncurses
  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  srand(time(NULL));

  game_init_medium_mode(&g_game);

  if (DEBUG_GAME_BOARD_SHOW_ALL) game_board_show_all(&g_game.game_board);

  ui_init(&ui);

  render_init(&ui.window_manager);

#if DEBUG_ENABLE_TEST
  debug_init();
#endif

  // Loop to track cursor position
  while (true) {
    struct Terminal* terminal = &ui.terminal;
    terminal_init(terminal);
    log_info_f("terminal={width:%d, height:%d}", terminal->width, terminal->height);
    struct Vector center = terminal_center(terminal);

    if (terminal->height < TERMINAL_MIN_HEIGHT) {
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

    render(center, &ui);

    enum GameState game_state = input_update(&g_game, &ui);
    if (game_state < GAME_STATE_MAX) {
      g_game.game_state = game_state;
    }

    game_print_state(g_game.game_state);
    if (game_state == GAME_STATE_QUIT) {
      break;
    }
  }

  endwin();  // End ncurses.
  return 0;
}

