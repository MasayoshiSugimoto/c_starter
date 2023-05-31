#ifndef RENDER_H
#define RENDER_H


void render_game_over_init() {
  struct GameWindow* game_window = &g_game_windows[GAME_WINDOW_ID_GAME_OVER];
  game_window->width = 13;
  game_window->height = 3;
}


void render_game_over(struct GameBoard* game_board) {
  struct GameWindow* game_window = &g_game_windows[GAME_WINDOW_ID_GAME_OVER];
  mvwaddstr(game_window->window, 1, 1, " GAME OVER ");
}


void render_game_won_init() {
  struct GameWindow* game_window = &g_game_windows[GAME_WINDOW_ID_GAME_WON];
  game_window->width = 11;
  game_window->height = 3;
}


void render_game_won(struct GameBoard* game_board, int left, int top) {
  struct GameWindow* game_window = &g_game_windows[GAME_WINDOW_ID_GAME_WON];
  mvwaddstr(game_window->window, 1, 1, " YOU WON ");
}


#endif

