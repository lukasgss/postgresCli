#pragma once

struct conn_info {
  char *host;
  char *dbname;
  char *user;
  char *password;
};

struct conn_info parse_flags(int argc, char **argv);
void print_usage(void);
