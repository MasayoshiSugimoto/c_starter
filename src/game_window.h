#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H


/**
 * Order of this enum will define the Z axis when rendered.
 */
enum GameWindowId {
  GAME_WINDOW_ID_GAME_OVER,
  GAME_WINDOW_ID_GAME_WON,
  GAME_WINDOW_ID_MENU,
  GAME_WINDOW_ID_GAME_MENU,
  GAME_WINDOW_ID_MAX
};


struct GameWindow {
  WINDOW* window;
  int width;
  int height;
  int left;
  int top;
  bool enable;
};


struct GameWindow g_game_windows[GAME_WINDOW_ID_MAX];


void game_window_init() {
  for (int i = 0; i < GAME_WINDOW_ID_MAX; i++) {
    struct GameWindow* game_window = &g_game_windows[i];
    game_window->width = 10;
    game_window->height = 10;
    game_window->left = 0;
    game_window->top = 0;
    game_window->enable = true;
    game_window->window = newwin(
        game_window->height,
        game_window->width,
        game_window->left,
        game_window->top
    );
  }
}


/**
 * Prepare the rendering of all windows. Content of the window should be set
 * after this function is called.
 */
void game_window_prepare() {
  for (int i = 0; i < GAME_WINDOW_ID_MAX; i++) {
    struct GameWindow* game_window = &g_game_windows[i];
    if (!game_window->enable) continue;
    WINDOW* window = game_window->window;
    mvwin(window, game_window->top, game_window->left);
    wresize(window, game_window->height, game_window->width);
    box(window, 0, 0);
  }
}


/**
 * Render windows. This function should be called after the content of each
 * window is set.
 */
void game_window_render() {
  for (int i = 0; i < GAME_WINDOW_ID_MAX; i++) {
    struct GameWindow* game_window = &g_game_windows[i];
    if (!game_window->enable) continue;
    wrefresh(game_window->window);
  }
}


void game_window_enable_only(enum GameWindowId game_window_id) {
  for (int i = 0; i < GAME_WINDOW_ID_MAX; i++) {
    g_game_windows[i].enable = false;
  }
  g_game_windows[game_window_id].enable = true;
}


void game_window_disable_all() {
  for (int i = 0; i < GAME_WINDOW_ID_MAX; i++) {
    g_game_windows[i].enable = false;
  }
}


void game_window_erase() {
  for (int i = 0; i < GAME_WINDOW_ID_MAX; i++) {
    werase(g_game_windows[i].window);
  }
}


#endif

