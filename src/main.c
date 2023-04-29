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
#define BOARD_CELL_TYPE_HIDDEN '='
#define BOARD_CELL_TYPE_MINE 'M'
#define BOARD_CELL_TYPE_OK_MARKER 'O'
#define BOARD_CELL_TYPE_MINE_MARKER 'X'

#define DEFAULT_TEXT_BUF 512

#define DEBUG_GAME_BOARD_SHOW_ALL false
#define DEBUG_ENABLE_TEST false


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


void game_board_render(struct GameBoard* game_board) {
  int width = game_board->width;
  int height = game_board->height;
  char* board = game_board->board;
  bool* visibility_map = game_board->visibility_map;
  char* markers = game_board->markers;

  addch('+');
  for (int x = 0; x < width; x++) {
    addch('-');
  }
  addch('+');
  addch('\n');

  for (int y = 0; y < height; y++) {
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
    addch('\n');
  }

  addch('+');
  for (int x = 0; x < width; x++) {
    addch('-');
  }
  addch('+');
  addch('\n');
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

/********************************************************************************
* END GameBoard
********************************************************************************/

/********************************************************************************
* BEGIN Cursor
********************************************************************************/


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
* BEGIN GameBoard
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


void test() {
  struct Game game;
  struct GameBoard* game_board = &game.game_board;
  while (true) {
    game_init(&game, 10, 10);
    game_board_setup_game(game_board, 10);
    game_board_play_cell(game_board, 0, 0);
  }
}


int main() {
  log_init();

  if (DEBUG_ENABLE_TEST) test();

  // Initialize ncurses
  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  srand(time(NULL));
  term_get_size();

  struct Game game;
  struct GameBoard* game_board = &game.game_board;
  struct Cursor* cursor = &game.cursor;

  int width = 12;
  int height = 8;
  game_init(&game, width, height);

  if (DEBUG_GAME_BOARD_SHOW_ALL) game_board_show_all(game_board);
  int bomb_pourcentage = 10;
  game_board_setup_game(game_board, bomb_pourcentage);

  int cursor_x_offset = 1;
  int cursor_y_offset = 1;

  // Loop to track cursor position
  while (true) {
    {  // Update and render.
      move(0, 0);
      game_board_render(game_board);
      move(cursor->y + cursor_y_offset, cursor->x + cursor_x_offset);
      refresh();
    }

    {  // Handle user inputs.
      int c = getch();
      bool is_space_pressed = false;
      bool is_quit = false;
      switch (c) {
        case KEY_DOWN:
          cursor->y++;
          cursor_dump(cursor);
          break;
        case KEY_UP:
          cursor->y--;
          cursor_dump(cursor);
          break;
        case KEY_LEFT:
          cursor->x--;
          cursor_dump(cursor);
          break;
        case KEY_RIGHT:
          cursor->x++;
          cursor_dump(cursor);
          break;
        case ' ':
          log_info("Space key pressed.");
          is_space_pressed = true;
          break;
        case 'q':
          log_info("Q key pressed.");
          is_quit = true;
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

      if (is_space_pressed) {
        game_board_play_cell(game_board, cursor->x, cursor->y);
      }

      if (is_quit) {
        break;
      }
    }
  }

  endwin();  // End ncurses.
  return 0;
}


