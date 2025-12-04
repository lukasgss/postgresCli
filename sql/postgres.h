#pragma once

#include "libpq-fe.h"
PGconn *connect_to_db(char *host, char *db, char *user, char *password);

void execute_statement(PGconn *conn, char *statement);
void terminate_connection(PGconn *conn);

void get_db_metadata(PGconn *conn);
void clear_db_metadata();
