#ifndef GAME_BOARD_H
#define GAME_BOARD_H


#include "util.h"
#include "cursor.h"


#define BOARD_CELL_TYPE_EMPTY ' '
#define BOARD_CELL_TYPE_HIDDEN ACS_CKBOARD
#define BOARD_CELL_TYPE_MINE 'M'
#define BOARD_CELL_TYPE_OK_MARKER 'O'
#define BOARD_CELL_TYPE_MINE_MARKER 'X'


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
  addch(ACS_ULCORNER);
  for (int x = 0; x < width; x++) {
    addch(ACS_HLINE);
  }
  addch(ACS_URCORNER);

  line++;

  for (int y = 0; y < height; y++) {
    move(line, left);
    addch(ACS_VLINE);
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
    addch(ACS_VLINE);
    line++;
  }

  move(line, left);
  addch(ACS_LLCORNER);
  for (int x = 0; x < width; x++) {
    addch(ACS_HLINE);
  }
  addch(ACS_LRCORNER);
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


#endif
