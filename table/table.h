#pragma once

struct print_table_info {
  char **cols;
  char ***rows; // rows[row][col]

  int amount_cols;
  int amount_rows;
};

void draw_table(const struct print_table_info *print_info);
