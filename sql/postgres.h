#pragma once

#include "libpq-fe.h"

struct database_metadata {
  char **tables;
  size_t amount_tables;
};

PGconn *connect_to_db(char *host, char *db, char *user, char *password);

void execute_statement(PGconn *conn, char *statement);
void terminate_connection(PGconn *conn);

void fetch_db_metadata(PGconn *conn);
void clear_db_metadata();

struct database_metadata const *get_db_metadata(void);
