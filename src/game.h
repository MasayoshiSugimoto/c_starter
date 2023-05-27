#ifndef GAME_H
#define GAME_H


#include "render.h"


#define BOMB_POURCENTAGE 10


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


void game_init_easy_mode(struct Game* game) {
  int width = 8;
  int height = 4;
  game_init(game, width, height);
  game_board_setup_game(&game->game_board, BOMB_POURCENTAGE);
}


void game_init_medium_mode(struct Game* game) {
  int width = 16;
  int height = 8;
  game_init(game, width, height);
  game_board_setup_game(&game->game_board, BOMB_POURCENTAGE);
}


void game_init_hard_mode(struct Game* game) {
  int width = 32;
  int height = 16;
  game_init(game, width, height);
  game_board_setup_game(&game->game_board, BOMB_POURCENTAGE);
}

#endif
