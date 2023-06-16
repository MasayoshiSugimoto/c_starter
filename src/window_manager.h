#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H


#include <curses.h>
#include <ncurses.h>


enum WindowId {
  WINDOW_ID_GAME_OVER,
  WINDOW_ID_GAME_WON,
  WINDOW_ID_MENU,
  WINDOW_ID_GAME_MENU,
  WINDOW_ID_MAX
};


void window_manager_init();
void window_manager_set_left(enum WindowId window_id, int left);
void window_manager_set_top(enum WindowId window_id, int top);
int window_manager_get_width(enum WindowId window_id);
void window_manager_set_width(enum WindowId window_id, int width);
int window_manager_get_height(enum WindowId window_id);
void window_manager_set_height(enum WindowId window_id, int height);
WINDOW* window_manager_get_window(enum WindowId window_id);
void window_manager_erase();
WINDOW* window_manager_setup_window(
    enum WindowId window_id,
    int center_x,
    int center_y
);
void window_manager_render();


#endif
