#include "render.h"


void render_help_menu() {
  mvaddstr(0, 0, "Press `Esc` to display the menu.");
}


void render_game_board(struct GameBoard* game_board, int left, int top) {
  int width = game_board->width;
  int height = game_board->height;
  char* board = game_board->board;
  bool* visibility_map = game_board->visibility_map;
  char* markers = game_board->markers;

  int line = top;
  move(line, left);
  addch(ACS_ULCORNER);
  for (int x = 0; x < width; x++) {
    addch(ACS_HLINE);
  }
  addch(ACS_URCORNER);

  line++;

  for (int y = 0; y < height; y++) {
    move(line, left);
    addch(ACS_VLINE);
    for (int x = 0; x < width; x++) {
      int i = game_board_get_index(game_board, x, y);
      if (visibility_map[i]) {
        if (board[i] == BOARD_CELL_TYPE_MINE) {
          addch(BOARD_CELL_TYPE_MINE);
        } else if (board[i] == BOARD_CELL_TYPE_EMPTY) {
          addch(BOARD_CELL_TYPE_EMPTY);
        } else {
          addch((char)'0' + board[i]);
        }
      } else if (markers[i] == BOARD_CELL_TYPE_OK_MARKER) {
        addch(BOARD_CELL_TYPE_OK_MARKER);
      } else if (markers[i] == BOARD_CELL_TYPE_MINE_MARKER) {
        addch(BOARD_CELL_TYPE_MINE_MARKER);
      } else {
        addch(BOARD_CELL_TYPE_HIDDEN);
      }
    }
    addch(ACS_VLINE);
    line++;
  }

  move(line, left);
  addch(ACS_LLCORNER);
  for (int x = 0; x < width; x++) {
    addch(ACS_HLINE);
  }
  addch(ACS_LRCORNER);
}


void render_in_game(struct Game* game, struct Vector center) {
  struct GameBoard* game_board = &game->game_board;
  struct Cursor* cursor = &game->cursor;
  int game_board_left = center.x - (game_board->width + 2) / 2;
  int game_board_top = center.y - (game_board->height + 2) / 2;

  render_game_board(game_board, game_board_left, game_board_top);

  const int border = 1;
  move(
      cursor->y + border + game_board_top,
      cursor->x + border + game_board_left
  );
}


void render_game_menu(
    struct GameMenu* game_menu,
    struct WindowManager* window_manager,
    int center_x,
    int center_y
) {
  enum WindowId id = WINDOW_ID_GAME_MENU;
  WINDOW* window = window_manager_setup_window(
      window_manager,
      id,
      center_x,
      center_y
  );

  char* title = "MENU";
  int text_x = (window_manager_get_width(window_manager, id) - strlen(title)) / 2;
  int text_y = 2;
  mvwaddstr(window, text_y, text_x, title);
  mvwaddstr(window, text_y + 1, text_x, "====");

  text_x = 12;
  text_y += 3;
  int space_y = 2;
  mvwaddstr(window, text_y, text_x, "Resume");
  mvwaddstr(window, text_y + space_y, text_x, "New Game");
  mvwaddstr(window, text_y + space_y * 2, text_x, "Manual");
  mvwaddstr(window, text_y + space_y * 3, text_x, "Quit");

  // Render cursor.
  mvwaddch(window, text_y + (game_menu->selected * space_y), 9, '>');
}


void render_menu(
    struct Menu* menu,
    struct WindowManager* window_manager,
    struct Vector center
) {
  WINDOW* window = window_manager_setup_window(
      window_manager,
      WINDOW_ID_MENU,
      center.x,
      center.y
  );


  int text_x = window_manager_get_width(window_manager, WINDOW_ID_MENU) / 2 - 11;
  int text_y = 3;
  mvwaddstr(window, text_y, text_x, "CHOOSE YOUR DIFFICULTY");
  mvwaddstr(window, text_y + 1, text_x, "======================");

  int start_x = 10;
  int start_y = 6;
  mvwaddstr(window, start_y + 0, start_x + 2, "Easy");
  mvwaddstr(window, start_y + 2, start_x + 2, "Medium");
  mvwaddstr(window, start_y + 4, start_x + 2, "Hard");

  // Render cursor.
  mvwaddch(window, start_y + (menu->menu_selection * 2), start_x, '>');
}


void render(struct Vector center, struct UI* ui, struct Game* game) {
  enum GameState game_state = game->game_state;
  struct WindowManager* window_manager = &ui->window_manager;
  WINDOW* window = NULL;

  erase();
  window_manager_erase(window_manager);
  render_help_menu();

  if (game_menu_is_enabled(&ui->game_menu)) {
    log_info("Game menu is enabled.");
    render_game_menu(&ui->game_menu, window_manager, center.x, center.y);

    curs_set(CURSOR_VISIBILITY_INVISIBLE);
    move(0, 0);
  } else if (game_state == GAME_STATE_IN_GAME) {
    render_in_game(game, center);
    curs_set(CURSOR_VISIBILITY_HIGH_VISIBILITY);
  } else if (game_state == GAME_STATE_GAME_OVER) {
    render_in_game(game, center);

    window = window_manager_setup_window(
        window_manager,
        WINDOW_ID_GAME_OVER,
        center.x,
        center.y
        );
    mvwaddstr(window, 1, 1, " GAME OVER ");

    curs_set(CURSOR_VISIBILITY_INVISIBLE);
    move(0, 0);
  } else if (game_state == GAME_STATE_GAME_WON) {
    render_in_game(game, center);

    window = window_manager_setup_window(
        window_manager,
        WINDOW_ID_GAME_WON,
        center.x,
        center.y
        );
    mvwaddstr(window, 1, 1, " YOU WON ");

    curs_set(CURSOR_VISIBILITY_INVISIBLE);
    move(0, 0);
  } else if (game_state == GAME_STATE_MENU) {
    render_menu(&ui->menu, window_manager, center);

    curs_set(CURSOR_VISIBILITY_INVISIBLE);
    move(0, 0);
  } else {
    log_fatal_f("Invalid game_state: %d", game_state);
  }

  refresh();
  window_manager_render(window_manager);
}

