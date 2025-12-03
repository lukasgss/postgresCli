#pragma once

struct print_table_info {
  char **cols;
  char **rows;

  int amount_cols;
  int amount_rows;
};

void draw_table(struct print_table_info print_info);
