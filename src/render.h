#ifndef RENDER_H
#define RENDER_H


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


void render_menu_state(struct Terminal* terminal, struct Menu* menu) {
  int left = terminal->width / 2 - menu_get_width(menu) / 2;
  int top = terminal->height / 2 - menu_get_height(menu) / 2;
  menu_set_position(menu, left, top);
  menu_erase(menu);
  menu_render(menu);

  curs_set(CURSOR_VISIBILITY_INVISIBLE);
  move(0, 0);
}


#endif

