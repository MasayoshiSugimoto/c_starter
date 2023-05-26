#ifndef CURSOR_H
#define CURSOR_H


#include "log.h"


enum CursorVisibility {
  CURSOR_VISIBILITY_INVISIBLE = 0,
  CURSOR_VISIBILITY_NORMAL = 1,
  CURSOR_VISIBILITY_HIGH_VISIBILITY = 2
};


struct Cursor {
  int x;
  int y;
};


void cursor_dump(struct Cursor* cursor) {
  log_info_f("cursor: x=%d, y=%d", cursor->x, cursor->y);
}


#endif

