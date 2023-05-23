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

/********************************************************************************
* Main
********************************************************************************/

#define BOARD_CELL_TYPE_EMPTY ' '
#define BOARD_CELL_TYPE_HIDDEN '#'
#define BOARD_CELL_TYPE_MINE 'M'
#define BOARD_CELL_TYPE_OK_MARKER 'O'
#define BOARD_CELL_TYPE_MINE_MARKER 'X'

#define DEFAULT_TEXT_BUF 512

#define DEBUG_GAME_BOARD_SHOW_ALL false
#define DEBUG_ENABLE_TEST false

#define TERMINAL_MIN_HEIGHT 14


#define array_size(arr) sizeof(arr) / sizeof(arr[0])


/********************************************************************************
* BEGIN Log
********************************************************************************/

#define DEBUG_FILE "/tmp/minesweeper.log"
struct _IO_FILE* g_debug_file;


#define log_info(text) { \
  fprintf(g_debug_file, "[INFO][%s:%d] ", __FILE__, __LINE__); \
  fprintf(g_debug_file, text); \
  fprintf(g_debug_file, "\n"); \
  fflush(g_debug_file); \
}


#define log_info_f(pattern, ...) { \
  fprintf(g_debug_file, "[INFO][%s:%d] ", __FILE__, __LINE__); \
  fprintf(g_debug_file, pattern, __VA_ARGS__); \
  fprintf(g_debug_file, "\n"); \
  fflush(g_debug_file); \
}


#define log_error(text) { \
  fprintf(g_debug_file, "[ERROR][%s:%d] ", __FILE__, __LINE__); \
  fprintf(g_debug_file, text); \
  fprintf(g_debug_file, "\n"); \
  fflush(g_debug_file); \
}


#define log_error_f(pattern, ...) { \
  fprintf(g_debug_file, "[ERROR][%s:%d] ", __FILE__, __LINE__); \
  fprintf(g_debug_file, pattern, __VA_ARGS__); \
  fprintf(g_debug_file, "\n"); \
  fflush(g_debug_file); \
}


#define log_fatal(text) { \
  fprintf(g_debug_file, "[FATAL][%s:%d] ", __FILE__, __LINE__); \
  fprintf(g_debug_file, text); \
  fprintf(g_debug_file, "\n"); \
  fflush(g_debug_file); \
  exit(1); \
}


#define log_fatal_f(pattern, ...) { \
  fprintf(g_debug_file, "[FATAL][%s:%d] ", __FILE__, __LINE__); \
  fprintf(g_debug_file, pattern, __VA_ARGS__); \
  fprintf(g_debug_file, "\n"); \
  fflush(g_debug_file); \
  exit(1); \
}


void log_init() {
  g_debug_file = fopen(DEBUG_FILE, "w+");
  if (g_debug_file == NULL) {
    log_fatal_f("fopen(\"%s\") failed (%d): %s\n", DEBUG_FILE, errno, strerror(errno));
  }
}


/********************************************************************************
* END Log
********************************************************************************/


/********************************************************************************
* BEGIN Vector
********************************************************************************/

struct Vector {
  int x;
  int y;
};


void vector_as_string(char* buf, struct Vector v) {
  sprintf(buf, "{x:%d, y:%d}", v.x, v.y); 
}

/********************************************************************************
* END Vector
********************************************************************************/


/********************************************************************************
* BEGIN StaticQueue
********************************************************************************/

struct StaticQueueInt {
  int size;
  int max_size;
  int* queue;
};


void static_queue_int_push(struct StaticQueueInt* queue, int x) {
  queue->queue[queue->size++] = x;
}


int static_queue_int_pop(struct StaticQueueInt* queue) {
  queue->size--;
  return queue->queue[queue->size];
}


void static_queue_int_dump(struct StaticQueueInt* queue) {
  log_info("StaticQueueInt: {");
  log_info_f("  size: %d", queue->size);
  log_info_f("  max_size: %d", queue->max_size);
  log_info("  queue: [");
  for (int i = 0; i < queue->size; i++) {
    log_info_f("    %d", queue->queue[i]);
  }
  log_info("  ]");
}


/********************************************************************************
* END StaticQueue
********************************************************************************/


/********************************************************************************
* BEGIN Cursor
********************************************************************************/


enum CursorVisibility {
  CURSOR_VISIBILITY_INVISIBLE = 0,
  CURSOR_VISIBILITY_NORMAL = 1,
  CURSOR_VISIBILITY_HIGH_VISIBILITY = 2
};


struct Cursor {
  int x;
  int y;
};


void cursor_dump(struct Cursor* cursor) {
  log_info_f("cursor: x=%d, y=%d", cursor->x, cursor->y);
}


/********************************************************************************
* END Cursor
********************************************************************************/


/********************************************************************************
* BEGIN GameBoard
********************************************************************************/


#define GAME_BOARD_WIDTH_MAX 64
#define GAME_BOARD_HEIGHT_MAX 64
#define GAME_BOARD_SIZE_MAX (GAME_BOARD_WIDTH_MAX * GAME_BOARD_HEIGHT_MAX)

struct GameBoard {
  int width;
  int height;
  char board[GAME_BOARD_SIZE_MAX];
  bool visibility_map[GAME_BOARD_SIZE_MAX];
  char markers[GAME_BOARD_SIZE_MAX];
};


void game_board_init(struct GameBoard* game_board, int width, int height) {
  if (width > GAME_BOARD_WIDTH_MAX) {
    log_fatal_f("width is bigger than the allowed max. width=%d", width);
  }

  if (height > GAME_BOARD_HEIGHT_MAX) {
    log_fatal_f("height is bigger than the allowed max. height=%d", height);
  }

  game_board->width = width;
  game_board->height = height;
  for (int i = 0; i < width * height; i++) {
    game_board->board[i] = BOARD_CELL_TYPE_EMPTY;
    game_board->visibility_map[i] = false;
    game_board->markers[i] = BOARD_CELL_TYPE_EMPTY;
  }
}


int game_board_max_index(struct GameBoard* game_board) {
  return game_board->width * game_board->height;
}


int game_board_get_index(struct GameBoard* game_board, int x, int y) {
  return y * game_board->width + x;
}


void game_board_render(struct GameBoard* game_board, int left, int top) {
  int width = game_board->width;
  int height = game_board->height;
  char* board = game_board->board;
  bool* visibility_map = game_board->visibility_map;
  char* markers = game_board->markers;

  int line = top;
  move(line, left);
  addch('+');
  for (int x = 0; x < width; x++) {
    addch('-');
  }
  addch('+');

  line++;

  for (int y = 0; y < height; y++) {
    move(line, left);
    addch('|');
    for (int x = 0; x < width; x++) {
      int i = game_board_get_index(game_board, x, y);
      if (visibility_map[i]) {
        if (board[i] == BOARD_CELL_TYPE_MINE) {
          addch(BOARD_CELL_TYPE_MINE);
        } else if (board[i] == BOARD_CELL_TYPE_EMPTY) {
          addch(BOARD_CELL_TYPE_EMPTY);
        } else {
          addch((char)'0' + board[i]);
        }
      } else if (markers[i] == BOARD_CELL_TYPE_OK_MARKER) {
        addch(BOARD_CELL_TYPE_OK_MARKER);
      } else if (markers[i] == BOARD_CELL_TYPE_MINE_MARKER) {
        addch(BOARD_CELL_TYPE_MINE_MARKER);
      } else {
        addch(BOARD_CELL_TYPE_HIDDEN);
      }
    }
    addch('|');
    line++;
  }

  move(line, left);
  addch('+');
  for (int x = 0; x < width; x++) {
    addch('-');
  }
  addch('+');
}


void game_board_set_mine(struct GameBoard* game_board, int x, int y) {
  game_board->board[game_board_get_index(game_board, x, y)] = BOARD_CELL_TYPE_MINE;
}


void game_board_show_cell(struct GameBoard* game_board, int x, int y) {
  game_board->visibility_map[game_board_get_index(game_board, x, y)] = true;
}


int game_board_get_line(struct GameBoard* game_board, int index) {
  return index / game_board->width;
}


int game_board_get_column(struct GameBoard* game_board, int index) {
  return index % game_board->width;
}


void game_board_setup_game(struct GameBoard* game_board, int pourcentage) {
  log_info_f("game_board_setup_game(game_board, %d)", pourcentage);
  int width = game_board->width;
  int height = game_board->height;
  int cell_count = game_board->width * game_board->height;
  char* board = game_board->board;
  int bomb_count = (int)(cell_count * pourcentage / 100);

  // Set random mines
  for (int i = 0; i < bomb_count; i++) {
    int x = rand() % width;
    int y = rand() % height;
    board[game_board_get_index(game_board, x, y)] = BOARD_CELL_TYPE_MINE;
  }

  // Set mine counters.
  int offsets[] = {
    -1,
    width - 1,
    -width - 1,
    1,
    width + 1,
    -width + 1,
    width,
    -width
  };
  for (int board_i = 0; board_i < cell_count && board_i < GAME_BOARD_SIZE_MAX; board_i++) {
    if (board[board_i] != BOARD_CELL_TYPE_MINE) continue;
    for (int offset_i = 0; offset_i < array_size(offsets); offset_i++) {
      int j = board_i + offsets[offset_i];
      if (j < 0 || j >= cell_count) continue;
      if (offset_i <= 2
          && (game_board_get_column(game_board, j) != game_board_get_column(game_board, board_i) - 1)
      ) continue;
      if (offset_i > 2
          && offset_i <= 5
          && game_board_get_column(game_board, j) != game_board_get_column(game_board, board_i) + 1
      ) continue;
      if (board[j] == BOARD_CELL_TYPE_MINE) continue;
      if (board[j] == BOARD_CELL_TYPE_EMPTY) {
        board[j] = 1;
      } else {
        board[j]++;
      }
    }
  }
}


void game_board_show_all(struct GameBoard* game_board) {
  for (int i = 0; i < game_board->width * game_board->height; i++) {
    game_board->visibility_map[i]  = true;
  }
}


void game_board_play_cell(struct GameBoard* game_board, int x, int y) {
  log_info_f("game_board_play_cell(game_board, %d, %d)", x, y);

  char* board = game_board->board;
  bool* visibility_map = game_board->visibility_map;
  int width = game_board->width;

  int offsets[] = {1, -1, width, -width};
  int cells[GAME_BOARD_SIZE_MAX];
  int cells_size = 1;

  cells[0] = game_board_get_index(game_board, x, y);
  for (int i = 0; i < cells_size && i < GAME_BOARD_SIZE_MAX; i++) {
    if (visibility_map[cells[i]]) continue;
    visibility_map[cells[i]] = true;
    if (board[cells[i]] != BOARD_CELL_TYPE_EMPTY) continue;
    for (int j = 0; j < array_size(offsets); j++) {
      int cell = cells[i] + offsets[j];
      if (cell < 0 || cell >= game_board_max_index(game_board)) continue;
      if (visibility_map[cell]) continue;
      if (
        j <= 1 
        && game_board_get_line(game_board, cell) != game_board_get_line(game_board, cells[i])
      ) continue;
      cells[cells_size++] = cell;
    }
  }

}


void game_board_switch_ok_marker(struct GameBoard* game_board, int x, int y) {
  log_info_f("game_board_switch_ok_marker(game_board, %d, %d)", x, y);
  char* markers = game_board->markers;
  int i = game_board_get_index(game_board, x, y);
  if (markers[i] == BOARD_CELL_TYPE_OK_MARKER) {
    markers[i] = BOARD_CELL_TYPE_HIDDEN;
  } else {
    markers[i] = BOARD_CELL_TYPE_OK_MARKER;
  }
}


void game_board_switch_mine_marker(struct GameBoard* game_board, int x, int y) {
  log_info_f("game_board_switch_mine_marker(game_board, %d, %d)", x, y);
  char* markers = game_board->markers;
  int i = game_board_get_index(game_board, x, y);
  if (markers[i] == BOARD_CELL_TYPE_MINE_MARKER) {
    markers[i] = BOARD_CELL_TYPE_HIDDEN;
  } else {
    markers[i] = BOARD_CELL_TYPE_MINE_MARKER;
  }
}

// Game is won if all hidden cells are mines.
bool game_board_is_win(struct GameBoard* game_board) {
  log_info("game_board_is_win(game_board)");
  char* board = game_board->board;
  bool* visibility_map = game_board->visibility_map;

  for (int i = 0; i < game_board_max_index(game_board); i++) {
    if (visibility_map[i] && board[i] == BOARD_CELL_TYPE_MINE) return false;
    if (!visibility_map[i] && board[i] != BOARD_CELL_TYPE_MINE) return false;
  }
  return true;
}


// Game is lost when a mine is visible.
bool game_board_is_lost(struct GameBoard* game_board) {
  log_info("game_board_is_lost(game_board)");
  char* board = game_board->board;
  bool* visibility_map = game_board->visibility_map;

  for (int i = 0; i < game_board_max_index(game_board); i++) {
    if (visibility_map[i] && board[i] == BOARD_CELL_TYPE_MINE) return true;
  }
  return false;
}


/*
 * Move the cursor but keep it inside the board.
 * If moved in diagonal, allow displacement on one axis if allowed.
 */
void game_board_move_cursor(
    struct GameBoard* game_board,
    struct Cursor* cursor,
    int x,
    int y
) {
  int newX = cursor->x + x;
  int newY = cursor->y + y;
  if (newX >= 0 && newX < game_board->width) {
    cursor->x = newX;
  }
  if (newY >= 0 && newY < game_board->height) {
    cursor->y = newY;
  }
}


/********************************************************************************
* END GameBoard
********************************************************************************/

/********************************************************************************
* BEGIN Game
********************************************************************************/


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


/********************************************************************************
* END Game
********************************************************************************/


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
* BEGIN Render
********************************************************************************/


struct Terminal {
  int width;
  int height;
};


void terminal_init(struct Terminal* terminal) {
  getmaxyx(stdscr, terminal->height, terminal->width);
}


struct Vector terminal_center(struct Terminal* terminal) {
  struct Vector v;
  v.x = terminal->width / 2;
  v.y = terminal->height / 2;
  return v;
}


/********************************************************************************
* END Render
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


void test(struct Game* game) {
}


enum GameState {
  GAME_STATE_IN_GAME,
  GAME_STATE_GAME_OVER,
  GAME_STATE_GAME_WON
};


struct Inputs {
  bool is_quit;
};


void input_init(struct Inputs* inputs) {
  inputs->is_quit = false;
}


void input_update_in_game(struct Inputs* inputs, struct Game* game) {
  struct GameBoard* game_board = &game->game_board;
  struct Cursor* cursor = &game->cursor;
  int c = getch();
  switch (c) {
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



void input_update_game_over(struct Inputs* inputs, struct Game* game) {
  int c = getch();
  switch (c) {
    case 'q':
      log_info("Q key pressed.");
      inputs->is_quit = true;
      break;
  }
}


void input_update_game_won(struct Inputs* inputs, struct Game* game) {
  int c = getch();
  switch (c) {
    case 'q':
      log_info("Q key pressed.");
      inputs->is_quit = true;
      break;
  }
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
  struct Cursor* cursor = &game.cursor;

  if (DEBUG_ENABLE_TEST) test(&game);

  int width = 15;
  int height = 9;
  game_init(&game, width, height);

  if (DEBUG_GAME_BOARD_SHOW_ALL) game_board_show_all(game_board);
  int bomb_pourcentage = 10;
  game_board_setup_game(game_board, bomb_pourcentage);

  int cursor_x_offset = 1;
  int cursor_y_offset = 1;

  enum GameState game_state = GAME_STATE_IN_GAME;

  struct Inputs inputs;
  input_init(&inputs);

  struct Terminal terminal;

  // Loop to track cursor position
  while (true) {
    terminal_init(&terminal);
    log_info_f("terminal={width:%d, height:%d}", terminal.width, terminal.height);
    struct Vector center = terminal_center(&terminal);

    game_state = GAME_STATE_IN_GAME;

    if (game_board_is_lost(game_board)) {
      game_state = GAME_STATE_GAME_OVER;
    } else if (game_board_is_win(game_board)) {
      game_state = GAME_STATE_GAME_WON;
    } else {
      game_state = GAME_STATE_IN_GAME;
    }

    {  // Update and render.
      erase();

      if (terminal.height < TERMINAL_MIN_HEIGHT) {
        log_info("Terminal height is less than minimum allowed.");
        addstr(
            "Terminal height is less than minimum allowed.\n"
            "Please resize the terminal.\n"
        );
        getch();
        continue;
      }

      int game_board_left = center.x - game_board->width / 2;
      int game_board_top = center.y - game_board->height / 2;

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
      refresh();
    }

    switch (game_state) {
      case GAME_STATE_IN_GAME:
        input_update_in_game(&inputs, &game);
        break;
      case GAME_STATE_GAME_OVER:
        input_update_game_over(&inputs, &game);
        game_init(&game, width, height);
        game_board_setup_game(game_board, bomb_pourcentage);
        break;
      case GAME_STATE_GAME_WON:
        input_update_game_won(&inputs, &game);
        break;
      default:
        log_fatal_f("Invalid game_state=%d", game_state);
    }

    if (inputs.is_quit) break;

  }

  endwin();  // End ncurses.
  return 0;
}

