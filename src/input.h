#ifndef INPUT_H
#define INPUT_H


#include "game_menu.h"


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


enum MenuCommand input_menu_update(struct Menu* menu, int input) {
  switch (input) {
    case KEY_DOWN:
      menu_move_cursor_down(menu);
      return MENU_COMMAND_DO_NOTHING;
    case KEY_UP:
      menu_move_cursor_up(menu);
      return MENU_COMMAND_DO_NOTHING;
    case KEY_RESIZE:
      return MENU_COMMAND_DO_NOTHING;
    default:
      return menu_command_from_selection(menu->menu_selection);
  }
}


enum GameMenuCommand input_game_menu_update(int input) {
  switch (input) {
    case KEY_DOWN:
      log_info("Down key pressed.");
      game_menu_move_cursor_down();
      return GAME_MENU_COMMAND_MAX;
    case KEY_UP:
      log_info("Up key pressed.");
      game_menu_move_cursor_up();
      return GAME_MENU_COMMAND_MAX;
    case KEY_RESIZE:
      log_info("Window resized.");
      return GAME_MENU_COMMAND_MAX;
    default:
      log_info_f("Key pressed: %d", input);
      return game_menu_get_selected();
  }
}


enum GameState input_update(struct Game* game, struct Menu* menu) {
  int input = getch();
  enum MenuCommand menu_command = MENU_COMMAND_DO_NOTHING;
  enum GameMenuCommand game_menu_command = GAME_MENU_COMMAND_MAX;
  enum GameState game_state = game->game_state;
  struct GameBoard* game_board = &game->game_board;

  if (game_menu_is_enabled()) {
    log_info("Game menu is enabled.");
    game_menu_command = input_game_menu_update(input);
    if (game_menu_command == GAME_MENU_QUIT) return GAME_STATE_QUIT;
    if (game_menu_command < GAME_MENU_COMMAND_MAX) {
      return game_menu_update_game_state(game_menu_command);
    }
  } else if (game_state == GAME_STATE_IN_GAME) {
    input_update_in_game(game, input);
    if (game_board_is_lost(game_board)) {
      return GAME_STATE_GAME_OVER;
    } else if (game_board_is_win(game_board)) {
      return GAME_STATE_GAME_WON;
    }
  } else if (game_state == GAME_STATE_GAME_OVER) {
    game_menu_enable();
  } else if (game_state == GAME_STATE_GAME_WON) {
    game_menu_enable();
  } else if (game_state == GAME_STATE_MENU) {
    menu_command = input_menu_update(menu, input);
    // State change based on events.
    switch (menu_command) {
      case MENU_COMMAND_SELECT_GAME_EASY:
        game_init_easy_mode(game);
        return GAME_STATE_IN_GAME;
      case MENU_COMMAND_SELECT_GAME_MEDIUM:
        game_init_medium_mode(game);
        return GAME_STATE_IN_GAME;
      case MENU_COMMAND_SELECT_GAME_HARD:
        game_init_hard_mode(game);
        return GAME_STATE_IN_GAME;
      case MENU_COMMAND_DO_NOTHING:
        break;
    }
  } else {
    log_fatal_f("Invalid game_state=%d", game_state);
  }

  return GAME_STATE_MAX;
}


#endif

