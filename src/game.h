#ifndef GAME_H
#define GAME_H


#define BOMB_POURCENTAGE 10


enum GameState {
  GAME_STATE_IN_GAME,
  GAME_STATE_GAME_OVER,
  GAME_STATE_GAME_WON,
  GAME_STATE_MENU,
  GAME_STATE_MAX
};


const char* g_game_state_strings[] = {
  "GAME_STATE_IN_GAME",
  "GAME_STATE_GAME_OVER",
  "GAME_STATE_GAME_WON",
  "GAME_STATE_MENU",
  "GAME_STATE_MAX"
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


void game_init_easy_mode(struct Game* game) {
  int width = 9;
  int height = 5;
  game_init(game, width, height);
  game_board_setup_game(&game->game_board, BOMB_POURCENTAGE);
}


void game_init_medium_mode(struct Game* game) {
  int width = 17;
  int height = 9;
  game_init(game, width, height);
  game_board_setup_game(&game->game_board, BOMB_POURCENTAGE);
}


void game_init_hard_mode(struct Game* game) {
  int width = 31;
  int height = 15;
  game_init(game, width, height);
  game_board_setup_game(&game->game_board, BOMB_POURCENTAGE);
}

#endif
