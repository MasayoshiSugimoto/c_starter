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

#define WIDTH 8
#define HEIGHT 8

#define BOARD_CELL_TYPE_EMPTY ' '
#define BOARD_CELL_TYPE_HIDDEN 'X'
#define BOARD_CELL_TYPE_MINE 'M'


#define DEFAULT_TEXT_BUF 512


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
* BEGIN Game
********************************************************************************/

char g_board[WIDTH * HEIGHT];
bool g_visible_map[WIDTH * HEIGHT];


void game_init() {
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    g_board[i] = BOARD_CELL_TYPE_EMPTY;
    g_visible_map[i] = false;
  }
}


int game_get_index(int x, int y) {
  return y * WIDTH + x;
}


void game_term_render() {
  addch('+');
  for (int x = 0; x < WIDTH; x++) {
    addch('-');
  }
  addch('+');
  addch('\n');

  for (int y = 0; y < HEIGHT; y++) {
    addch('|');
    for (int x = 0; x < WIDTH; x++) {
      int i = game_get_index(x, y);
      if (g_visible_map[i]) {
        if (g_board[i] == BOARD_CELL_TYPE_MINE) {
          addch(BOARD_CELL_TYPE_MINE);
        } else if(g_board[i] == BOARD_CELL_TYPE_EMPTY) {
          addch(BOARD_CELL_TYPE_EMPTY);
        } else {
          addch((char)'0' + g_board[i]);
        }
      } else {
        addch(BOARD_CELL_TYPE_HIDDEN);
      }
    }
    addch('|');
    addch('\n');
  }

  addch('+');
  for (int x = 0; x < WIDTH; x++) {
    addch('-');
  }
  addch('+');
  addch('\n');
}


void game_set_mine(int x, int y) {
  g_board[game_get_index(x, y)] = BOARD_CELL_TYPE_MINE;
}


void game_show_cell(int x, int y) {
  g_visible_map[game_get_index(x, y)] = true;
}


int game_get_line(int index) {
  return index / WIDTH;
}


int game_get_column(int index) {
  return index % WIDTH;
}


void game_setup_game(int pourcentage) {
  int cell_count = WIDTH * HEIGHT;
  int bomb_count = (int)(cell_count * pourcentage / 100);

  // Set random mines
  for (int i = 0; i < bomb_count; i++) {
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;
    int board_i = game_get_index(x, y);
    g_board[board_i] = BOARD_CELL_TYPE_MINE;
  }

  // Set mine counters.
  int offsets[] = {
    -1,
    WIDTH - 1,
    -WIDTH - 1,
    1,
    WIDTH + 1,
    -WIDTH + 1,
    WIDTH,
    -WIDTH
  };
  for (int board_i = 0; board_i < cell_count; board_i++) {
    if (g_board[board_i] != BOARD_CELL_TYPE_MINE) continue;
    for (int offset_i = 0; offset_i < 8; offset_i++) {
      int j = board_i + offsets[offset_i];
      if (offset_i <= 2 && game_get_column(j) != game_get_column(board_i) - 1) continue;
      if (
          offset_i > 2
          && offset_i <= 5
          && game_get_column(j) != game_get_column(board_i) + 1
      ) continue;
      if (j < 0) continue;
      if (j >= cell_count) continue;
      if (g_board[j] == BOARD_CELL_TYPE_MINE) continue;
      if (g_board[j] == BOARD_CELL_TYPE_EMPTY) {
        g_board[j] = 1;
      } else {
        g_board[j]++;
      }
    }
  }
}


void game_show_all() {
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    g_visible_map[i]  = true;
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

//
//int main() {
//  log_init();
//  log_info("Starting Minesweeper...");
//
//  srand(time(NULL));
//  term_get_size();
////  char buf[512];
////  int n;
////  float f;
////  int nb_read = 0;
////  while (true) {
////    nb_read = scanf("%s %d %f", buf, &n, &f);
////    log_info_f("nb_read=%d", nb_read);
////    if (nb_read < 0) break;
////    log_info_f("buf=%s, n=%d, f=%f", buf, n, f);
////  }
//
//  game_init();
//  int bomb_pourcentage = 10;
//  game_randomize(bomb_pourcentage);
//  game_render();
//
////  struct ttysize ts;
////  ioctl(0, TIOCGSIZE, &ts);
////  log_info_f("LINES: %s, COLUMNS: %s", ts.ts_lines, ts.ts_columns);
//
//  log_info("Minesweeper done.");
//	return 0;
//}


int main() {
  log_init();
  // Initialize ncurses
  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  srand(time(NULL));
  term_get_size();
  game_init();
  //game_show_all();
  int bomb_pourcentage = 10;
  game_setup_game(bomb_pourcentage);

  int x = 0;
  int y = 0;

  int cursor_x_offset = 1;
  int cursor_y_offset = 1;

  // Loop to track cursor position
  while (true) {
    {  // Update and render.
      move(0, 0);
      game_term_render();
      move(y + cursor_y_offset, x + cursor_x_offset);
      refresh();
    }

    {  // Handle user inputs.
      int c = getch();
      bool is_space_pressed = false;
      bool is_quit = false;
      switch (c) {
        case KEY_DOWN:
          y++;
          log_info_f("cursor: x=%d, y=%d", x, y);
          break;
        case KEY_UP:
          y--;
          log_info_f("cursor: x=%d, y=%d", x, y);
          break;
        case KEY_LEFT:
          x--;
          log_info_f("cursor: x=%d, y=%d", x, y);
          break;
        case KEY_RIGHT:
          x++;
          log_info_f("cursor: x=%d, y=%d", x, y);
          break;
        case ' ':
          log_info("Space key pressed.");
          is_space_pressed = true;
          break;
        case 'q':
          log_info("Q key pressed.");
          is_quit = true;
          break;
      }

      if (is_space_pressed) {
        game_show_cell(x, y);
      }

      if (is_quit) {
        break;
      }
    }
  }

  // End ncurses
  endwin();
  return 0;
}

