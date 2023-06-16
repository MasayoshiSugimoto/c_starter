#ifndef RENDER_H
#define RENDER_H


#include "window_manager.h"


void render_game_over_init() {
  window_manager_set_width(WINDOW_ID_GAME_OVER, 13);
  window_manager_set_height(WINDOW_ID_GAME_OVER, 3);
}


void render_game_won_init() {
  window_manager_set_width(WINDOW_ID_GAME_WON, 11);
  window_manager_set_height(WINDOW_ID_GAME_WON, 3);
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

  game_board_render(game_board, game_board_left, game_board_top);

  const int border = 1;
  move(
      cursor->y + border + game_board_top,
      cursor->x + border + game_board_left
  );
}


void render(struct Game* game, struct Vector center, struct Menu* menu) {
  enum GameState game_state = game->game_state;
  erase();
  window_manager_erase();
  render_help_menu();

  if (game_menu_is_enabled()) {
    log_info("Game menu is enabled.");
    game_menu_render(center.x, center.y);

    curs_set(CURSOR_VISIBILITY_INVISIBLE);
    move(0, 0);
  } else {
    WINDOW* window = NULL;
    switch (game_state) {
      case GAME_STATE_IN_GAME:
        render_in_game(game, game_state, center);
        curs_set(CURSOR_VISIBILITY_HIGH_VISIBILITY);
        break;
      case GAME_STATE_GAME_OVER:
        render_in_game(game, game_state, center);

        window = window_manager_setup_window(
            WINDOW_ID_GAME_OVER,
            center.x,
            center.y
        );
        mvwaddstr(window, 1, 1, " GAME OVER ");

        curs_set(CURSOR_VISIBILITY_INVISIBLE);
        move(0, 0);
        break;
      case GAME_STATE_GAME_WON:
        render_in_game(game, game_state, center);

        window = window_manager_setup_window(
            WINDOW_ID_GAME_WON,
            center.x,
            center.y
        );
        mvwaddstr(window, 1, 1, " YOU WON ");

        curs_set(CURSOR_VISIBILITY_INVISIBLE);
        move(0, 0);
        break;
      case GAME_STATE_MENU:
        window = window_manager_setup_window(
            WINDOW_ID_MENU,
            center.x,
            center.y
        );
        menu_render(menu, window);

        curs_set(CURSOR_VISIBILITY_INVISIBLE);
        move(0, 0);
        break;
      default:
        log_fatal_f("Invalid game_state: %d", game_state);
    }
  }
  refresh();
  window_manager_render();
}


#endif

