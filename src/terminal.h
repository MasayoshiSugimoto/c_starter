#ifndef TERMINAL_H
#define TERMINAL_H


struct Terminal {
  int width;
  int height;
};


void terminal_init(struct Terminal* terminal) {
  getmaxyx(stdscr, terminal->height, terminal->width);
}


struct Vector terminal_center(struct Terminal* terminal) {
  struct Vector v;
  v.x = terminal->width / 2;
  v.y = terminal->height / 2;
  return v;
}


#endif
