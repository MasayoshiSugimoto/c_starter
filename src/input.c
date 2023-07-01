#include "input.h"


void input_update_in_game(struct Game* game, int input) {
  struct GameBoard* game_board = &game->game_board;
  struct Cursor* cursor = &game->cursor;
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
      game_board_play_cell(game_board, cursor->x, cursor->y);
      break;
    case 'o':
      game_board_switch_ok_marker(game_board, cursor->x, cursor->y);
      break;
    case 'x':
      game_board_switch_mine_marker(game_board, cursor->x, cursor->y);
      break;
  }
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


void input_game_menu_update(
    int input,
    struct GameMenu* game_menu,
    struct Game* game
) {
  switch (input) {
    case KEY_DOWN:
      game_menu_move_cursor_down(game_menu);
      break;
    case KEY_UP:
      game_menu_move_cursor_up(game_menu);
      break;
    case KEY_RESIZE:
      break;
    default:
      switch (game_menu->selected) {
        case GAME_MENU_RESUME:
          log_info("Resuming game.");
          game_menu->enabled = false;
          break;
        case GAME_MENU_NEW_GAME:
          log_info("Starting new game.");
          game_menu->enabled = false;
          game_set_game_state(game, GAME_STATE_MENU);
          break;
        case GAME_MENU_MANUAL:
          log_info("Opening manual.");
          game_menu->enabled = false;
          break;
        case GAME_MENU_QUIT:
          log_info("Quiting...");
          game_set_game_state(game, GAME_STATE_QUIT);
          break;
        default:
          log_fatal_f("Invalid menu selection: %d", game_menu->selected);
      }
  }
}


void input_log_key_pressed(int input) {
  char* key = NULL;
  switch (input) {
    case KEY_DOWN:
      key = "KEY_DOWN";
      break;
    case KEY_UP:
      key = "KEY_UP";
      break;
    case KEY_RESIZE:
      key = "KEY_RESIZE";
      break;
    case KEY_LEFT:
      key = "KEY_LEFT";
      break;
    case KEY_RIGHT:
      key = "KEY_RIGHT";
      break;
  }
  if (key != NULL) {
    log_info_f("Key pressed: %s", key);
  } else {
    log_info_f("Key pressed: {unicode: %d, character: '%c'}", input, (char)input);
  }
}


void input_update(struct Game* game, struct UI* ui) {
  int input = getch();
  input_log_key_pressed(input);
  enum GameState game_state = game->game_state;
  struct GameBoard* game_board = &game->game_board;

  if (game_menu_is_enabled(&ui->game_menu)) {
    log_info("Game menu is enabled.");
    input_game_menu_update(input, &ui->game_menu, game);
  } else if (game_state == GAME_STATE_IN_GAME) {
    input_update_in_game(game, input);
    if (game_board_is_lost(game_board)) {
      game_set_game_state(game, GAME_STATE_GAME_OVER);
    } else if (game_board_is_win(game_board)) {
      game_set_game_state(game, GAME_STATE_GAME_WON);
    }
  } else if (game_state == GAME_STATE_GAME_OVER) {
    game_menu_enable(&ui->game_menu);
  } else if (game_state == GAME_STATE_GAME_WON) {
    game_menu_enable(&ui->game_menu);
  } else if (game_state == GAME_STATE_MENU) {
    if (input_menu_update(&ui->menu, input, game)) {
      game_set_game_state(game, GAME_STATE_IN_GAME);
    }
  } else {
    log_fatal_f("Invalid game_state=%d", game_state);
  }
}

