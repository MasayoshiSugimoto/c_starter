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

/********************************************************************************
* Main
********************************************************************************/

#define DEFAULT_TEXT_BUF 512

#define DEBUG_GAME_BOARD_SHOW_ALL false
#define DEBUG_ENABLE_TEST true

#define TERMINAL_MIN_HEIGHT 14


/********************************************************************************
* BEGIN Render
********************************************************************************/


void render_game_over(struct GameBoard* game_board, int left, int top) {
  mvaddstr(top, left, "           ");
  mvaddstr(top + 1, left, " GAME OVER ");
  mvaddstr(top + 2, left, "           ");
}


void render_game_won(struct GameBoard* game_board, int left, int top) {
  mvaddstr(top, left, "         ");
  mvaddstr(top + 1, left, " YOU WON ");
  mvaddstr(top + 2, left, "         ");
}


/********************************************************************************
* END Render
********************************************************************************/


/********************************************************************************
* BEGIN Game
********************************************************************************/


enum GameState {
  GAME_STATE_IN_GAME,
  GAME_STATE_GAME_OVER,
  GAME_STATE_GAME_WON,
  GAME_STATE_MENU
};


struct Game {
  struct GameBoard game_board;
  struct Cursor cursor; 
};


void game_init(struct Game* game, int width, int height) {
  log_info_f("game_init(game, %d, %d)", width, height);
  game->cursor.x = 0;
  game->cursor.y = 0;
  game_board_init(&game->game_board, width, height);
}


void game_render_in_game(
    struct Game* game,
    enum GameState game_state,
    struct Vector center
) {
  struct GameBoard* game_board = &game->game_board;
  struct Cursor* cursor = &game->cursor;
  int game_board_left = center.x - (game_board->width + 2) / 2;
  int game_board_top = center.y - (game_board->height + 2) / 2;
  const int cursor_x_offset = 1;
  const int cursor_y_offset = 1;

  game_board_render(game_board, game_board_left, game_board_top);

  move(
      cursor->y + cursor_y_offset + game_board_top,
      cursor->x + cursor_x_offset + game_board_left
  );
  switch (game_state) {
    case GAME_STATE_IN_GAME:
      curs_set(CURSOR_VISIBILITY_HIGH_VISIBILITY);
      break;
    case GAME_STATE_GAME_OVER:
      curs_set(CURSOR_VISIBILITY_INVISIBLE);
      render_game_over(
          game_board,
          game_board_left + game_board->width / 2 - 4,
          game_board_top + game_board->height / 2
          );
      break;
    case GAME_STATE_GAME_WON:
      curs_set(CURSOR_VISIBILITY_INVISIBLE);
      render_game_won(
          game_board,
          game_board_left + game_board->width / 2 - 3,
          game_board_top + game_board->height / 2
          );
      break;
    default:
      log_fatal_f("Invalid game_state=%d", game_state);
  }
}


/********************************************************************************
* END Game
********************************************************************************/


struct Vector term_get_size(void) {
  struct Vector empty_vector;
  empty_vector.x = 0;
  empty_vector.y = 0;

  char const *const term = getenv("TERM");
  if (term == NULL) {
    log_fatal("TERM environment variable not set\n" );
    return empty_vector;
  }
  log_info_f("TERM=%s", term);

  char const *const cterm_path = ctermid(NULL);
  if (cterm_path == NULL || cterm_path[0] == '\0') {
    log_fatal("ctermid() failed\n");
    return empty_vector;
  }
  log_info_f("cterm_path=%s", cterm_path);

  int tty_fd = open(cterm_path, O_RDWR);
  if (tty_fd == -1) {
    log_fatal_f("open(\"%s\") failed (%d): %s\n", cterm_path, errno, strerror(errno));
    return empty_vector;
  }

  int setupterm_err;
  if (setupterm((char*)term, tty_fd, &setupterm_err) == ERR) {
    switch (setupterm_err) {
      case -1:
        log_fatal("setupterm() failed: terminfo database not found\n");
        goto cleanup;
      case 0:
        log_fatal_f("setupterm() failed: TERM=%s not found in database\n", term);
        goto cleanup;
      case 1:
        log_fatal("setupterm() failed: terminal is hardcopy\n");
        goto cleanup;
    } // switch
  }

  int cols = tigetnum("cols");
  if (cols < 0) {
    log_fatal_f("tigetnum(\"cols\") failed (%d)\n", cols);
  }

  int l = tigetnum("lines");
  if (l < 0) {
    log_fatal_f("tigetnum(\"lines\") failed (%d)\n", l);
  }

cleanup:
  if (tty_fd != -1) close(tty_fd);
  struct Vector v;
  v.x = cols < 0 ? 0 : cols;
  v.y = l < 0 ? 0 : l;

  char buf[256];
  vector_as_string(buf, v);
  log_info_f("Terminal size: %s", buf);
  return v;
}


struct Inputs {
  bool is_quit;
};


void input_init(struct Inputs* inputs) {
  inputs->is_quit = false;
}


void input_update_in_game(struct Inputs* inputs, struct Game* game, int input) {
  struct GameBoard* game_board = &game->game_board;
  struct Cursor* cursor = &game->cursor;
  switch (input) {
    case KEY_DOWN:
      game_board_move_cursor(game_board, cursor, 0, 1);
      cursor_dump(cursor);
      break;
    case KEY_UP:
      game_board_move_cursor(game_board, cursor, 0, -1);
      cursor_dump(cursor);
      break;
    case KEY_LEFT:
      game_board_move_cursor(game_board, cursor, -1, 0);
      cursor_dump(cursor);
      break;
    case KEY_RIGHT:
      game_board_move_cursor(game_board, cursor, 1, 0);
      cursor_dump(cursor);
      break;
    case ' ':
      log_info("Space key pressed.");
      game_board_play_cell(game_board, cursor->x, cursor->y);
      break;
    case 'q':
      log_info("Q key pressed.");
      inputs->is_quit = true;
      break;
    case 'o':
      log_info("O key pressed.");
      game_board_switch_ok_marker(game_board, cursor->x, cursor->y);
      break;
    case 'x':
      log_info("X key pressed.");
      game_board_switch_mine_marker(game_board, cursor->x, cursor->y);
      break;
  }
}



void input_update_game_over(struct Inputs* inputs, struct Game* game, int input) {
  switch (input) {
    case 'q':
      log_info("Q key pressed.");
      inputs->is_quit = true;
      break;
  }
}


void input_update_game_won(struct Inputs* inputs, struct Game* game, int input) {
  switch (input) {
    case 'q':
      log_info("Q key pressed.");
      inputs->is_quit = true;
      break;
  }
}


#if DEBUG_ENABLE_TEST

struct DebugData {
  WINDOW *window;
} g_debug_data;


struct Menu menu;


void debug_init() {
  menu_init(&menu);
  menu_set_position(&menu, 1, 2);
}


void debug_loop() {
  menu_erase(&menu);
  curs_set(CURSOR_VISIBILITY_INVISIBLE);
  move(0, 0);
  menu_render(&menu);
  int input = getch();
  menu_update_input(&menu, input);
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

  int width = 15;
  int height = 9;
  game_init(&game, width, height);

  if (DEBUG_GAME_BOARD_SHOW_ALL) game_board_show_all(game_board);
  int bomb_pourcentage = 10;
  game_board_setup_game(game_board, bomb_pourcentage);

  enum GameState game_state = GAME_STATE_IN_GAME;

  struct Inputs inputs;
  input_init(&inputs);

  struct Terminal terminal;

#if DEBUG_ENABLE_TEST
  debug_init();
#endif

  // Loop to track cursor position
  while (true) {

    terminal_init(&terminal);
    log_info_f("terminal={width:%d, height:%d}", terminal.width, terminal.height);
    struct Vector center = terminal_center(&terminal);

    // Update game state.
    game_state = GAME_STATE_IN_GAME;
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
      case GAME_STATE_MENU:
        // State change based on events.
        break;
      default:
        log_fatal_f("Invalid game_state: %d", game_state);
    }

    {  // Update and render.
      erase();

      if (terminal.height < TERMINAL_MIN_HEIGHT) {
        log_info("Terminal height is less than the minimum allowed.");
        addstr(
            "Terminal height is less than the minimum allowed.\n"
            "Please resize the terminal.\n"
        );
      } else {
        game_render_in_game(&game, game_state, center);
      }

      refresh();
    }

    int input = getch();
    switch (game_state) {
      case GAME_STATE_IN_GAME:
        input_update_in_game(&inputs, &game, input);
        break;
      case GAME_STATE_GAME_OVER:
        input_update_game_over(&inputs, &game, input);
        game_init(&game, width, height);
        game_board_setup_game(game_board, bomb_pourcentage);
        break;
      case GAME_STATE_GAME_WON:
        input_update_game_won(&inputs, &game, input);
        break;
      default:
        log_fatal_f("Invalid game_state=%d", game_state);
    }

    if (inputs.is_quit) break;

  }

  endwin();  // End ncurses.
  return 0;
}

