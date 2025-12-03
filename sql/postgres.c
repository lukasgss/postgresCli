#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libpq-fe.h"

#define NANOSECONDS_PER_SEC 1000000000.0

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

static double measure_statement_exec_time_in_secs(
    PGresult *(func)(PGconn *, char *), PGconn *conn, char *statement,
    PGresult **result)
{
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    *result = func(conn, statement);

    clock_gettime(CLOCK_MONOTONIC, &end);

    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / NANOSECONDS_PER_SEC;
}

static inline PGresult *exec_to_db(PGconn *conn, char *statement)
{
    return PQexec(conn, statement);
}

void execute_statement(PGconn *conn, char *statement)
{
    // can't create a value if not a pointer, since type
    // isn't defined in the header file
    PGresult *result = NULL;

    double elapsed_time = measure_statement_exec_time_in_secs(
        exec_to_db, conn, statement, &result);

    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK)
    {
        fprintf(
            stderr, "Failed to execute statement: %s\n", PQerrorMessage(conn));
        PQclear(result);
        exit(EXIT_FAILURE);
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
