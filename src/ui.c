#include "ui.h"


void ui_init(struct UI* ui) {
  menu_init(&ui->menu, &ui->window_manager);
  game_menu_init(&ui->game_menu);
}
