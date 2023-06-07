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


void render_game_won(struct GameBoard* game_board) {
  struct GameWindow* game_window = &g_game_windows[GAME_WINDOW_ID_GAME_WON];
  mvwaddstr(game_window->window, 1, 1, " YOU WON ");
}


void render_help_menu() {
  mvaddstr(0, 0, "Press `Esc` to display the menu.");
}


void render_in_game(
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
}


void render(
    enum GameState game_state,
    struct Game* game,
    struct Vector center,
    struct Menu* menu
) {
  erase();
  game_window_erase();
  render_help_menu();

  if (game_menu_is_enabled()) {
    log_info("Game menu is enabled.");
    game_window_enable_only(GAME_WINDOW_ID_GAME_MENU);
    game_menu_render(center.x, center.y);

    curs_set(CURSOR_VISIBILITY_INVISIBLE);
    move(0, 0);
  } else {
    switch (game_state) {
      case GAME_STATE_IN_GAME:
        render_in_game(game, game_state, center);
        game_window_disable_all();
        curs_set(CURSOR_VISIBILITY_HIGH_VISIBILITY);
        break;
      case GAME_STATE_GAME_OVER:
        render_in_game(game, game_state, center);

        main_setup_window(GAME_WINDOW_ID_GAME_OVER, center);
        render_game_over(&game->game_board);

        curs_set(CURSOR_VISIBILITY_INVISIBLE);
        move(0, 0);
        break;
      case GAME_STATE_GAME_WON:
        render_in_game(game, game_state, center);

        main_setup_window(GAME_WINDOW_ID_GAME_WON, center);
        render_game_won(&game->game_board);

        curs_set(CURSOR_VISIBILITY_INVISIBLE);
        move(0, 0);
        break;
      case GAME_STATE_MENU:
        main_setup_window(GAME_WINDOW_ID_MENU, center);
        menu_render(menu);

        curs_set(CURSOR_VISIBILITY_INVISIBLE);
        move(0, 0);
        break;
      default:
        log_fatal_f("Invalid game_state: %d", game_state);
    }
  }
  refresh();
  game_window_render();
}


#endif

