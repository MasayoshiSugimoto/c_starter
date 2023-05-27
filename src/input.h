#ifndef INPUT_H
#define INPUT_H


struct Inputs {
  bool is_quit;
};


void input_init(struct Inputs* inputs) {
  inputs->is_quit = false;
}


void input_update_in_game(struct Inputs* inputs, struct Game* game, int input) {
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
      log_info("Space key pressed.");
      game_board_play_cell(game_board, cursor->x, cursor->y);
      break;
    case 'q':
      log_info("Q key pressed.");
      inputs->is_quit = true;
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
}


void input_update_game_over(struct Inputs* inputs, struct Game* game, int input) {
  switch (input) {
    case 'q':
      log_info("Q key pressed.");
      inputs->is_quit = true;
      break;
  }
}


void input_update_game_won(struct Inputs* inputs, struct Game* game, int input) {
  switch (input) {
    case 'q':
      log_info("Q key pressed.");
      inputs->is_quit = true;
      break;
  }
}


#endif

