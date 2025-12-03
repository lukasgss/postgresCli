#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libpq-fe.h"

static void exit_gracefully(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

PGconn *connect_to_db(char *host, char *db, char *user, char *password)
{
    if (user == NULL)
    {
        exit_gracefully(NULL);
    }

    char conn_str[512];
    if (password == NULL)
    {
        if (db == NULL || strcmp(db, "") == 0)
        {
            snprintf(
                conn_str, sizeof conn_str, "dbname=%s user=%s", host, user);
        }
        else
        {
            snprintf(
                conn_str,
                sizeof conn_str,
                "host=%s dbname=%s user=%s",
                host,
                db,
                user);
        }
    }
    else
    {
        if (db == NULL || strcmp(db, "") == 0)
        {
            snprintf(
                conn_str, sizeof conn_str, "dbname=%s user=%s", host, user);
        }
        else
        {
            snprintf(
                conn_str,
                sizeof conn_str,
                "host=%s dbname=%s user=%s password=%s",
                host,
                db,
                user,
                password);
        }
    }

    PGconn *conn = PQconnectdb(conn_str);
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection failed: %s", PQerrorMessage(conn));
        exit_gracefully(conn);
    }

    return conn;
}

void execute_statement(PGconn *conn, char *statement)
{
    PGresult *result = PQexec(conn, statement);
    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK)
    {
        fprintf(
            stderr, "Failed to execute statement: %s\n", PQerrorMessage(conn));
        PQclear(result);
    }

    int num_rows = PQntuples(result);
    int num_cols = PQnfields(result);

    for (int col = 0; col < num_cols; col++)
    {
        printf("\n%s\t", PQfname(result, col));
    }
    printf("\n");

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            char *value = PQgetvalue(result, row, col);
            printf("%s\t", value);
        }

        printf("\n");
    }

    PQclear(result);
}

void terminate_connection(PGconn *conn) { PQfinish(conn); }
