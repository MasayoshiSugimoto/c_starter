#include "input.h"


bool input_update_in_game(struct Game* game, int input) {
  struct GameBoard* game_board = &game->game_board;
  struct Cursor* cursor = &game->cursor;
  bool is_quit = false;
  switch (input) {
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
    case 'o':
      log_info("O key pressed.");
      game_board_switch_ok_marker(game_board, cursor->x, cursor->y);
      break;
    case 'x':
      log_info("X key pressed.");
      game_board_switch_mine_marker(game_board, cursor->x, cursor->y);
      break;
  }
  return is_quit;
}


bool input_menu_update(struct Menu* menu, int input, struct Game* game) {
  bool is_new_game = false;
  switch (input) {
    case KEY_DOWN:
      menu_move_cursor_down(menu);
      break;
    case KEY_UP:
      menu_move_cursor_up(menu);
      break;
    case KEY_RESIZE:
      break;
    default:
      return menu_validate(menu, game);
  }
  return is_new_game;
}


enum GameState input_game_menu_update(
    int input,
    enum GameState game_state,
    struct GameMenu* game_menu
) {
  switch (input) {
    case KEY_DOWN:
      log_info("Down key pressed.");
      game_menu_move_cursor_down(game_menu);
      return GAME_STATE_MAX;
    case KEY_UP:
      log_info("Up key pressed.");
      game_menu_move_cursor_up(game_menu);
      return GAME_STATE_MAX;
    case KEY_RESIZE:
      log_info("Window resized.");
      return GAME_STATE_MAX;
    default:
      log_info_f("Key pressed: %d", input);
      return game_menu_validate(game_menu);
  }
}


enum GameState input_update(struct Game* game, struct UI* ui) {
  int input = getch();
  enum GameState game_state = game->game_state;
  struct GameBoard* game_board = &game->game_board;

  if (game_menu_is_enabled(&ui->game_menu)) {
    log_info("Game menu is enabled.");
    return input_game_menu_update(input, game_state, &ui->game_menu);
  } else if (game_state == GAME_STATE_IN_GAME) {
    input_update_in_game(game, input);
    if (game_board_is_lost(game_board)) {
      return GAME_STATE_GAME_OVER;
    } else if (game_board_is_win(game_board)) {
      return GAME_STATE_GAME_WON;
    }
  } else if (game_state == GAME_STATE_GAME_OVER) {
    game_menu_enable(&ui->game_menu);
  } else if (game_state == GAME_STATE_GAME_WON) {
    game_menu_enable(&ui->game_menu);
  } else if (game_state == GAME_STATE_MENU) {
    if (input_menu_update(&ui->menu, input, game)) return GAME_STATE_IN_GAME;
  } else {
    log_fatal_f("Invalid game_state=%d", game_state);
  }

  return GAME_STATE_MAX;
}

