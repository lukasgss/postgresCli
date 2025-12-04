#include <bits/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include <time.h>

#include "libpq-fe.h"
#include "table/table.h"

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

static __always_inline void free_results(char ***row_values, int num_rows)
{
    for (int row = 0; row < num_rows; row++)
    {
        free(row_values[row]);
    }

    free(row_values);
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
        fprintf(stderr, "\n%s", PQerrorMessage(conn));
        PQclear(result);
        return;
    }

    int num_rows = PQntuples(result);
    int num_cols = PQnfields(result);

    unsigned long biggest_str = 0;
    char *col_values[num_cols];

    for (int col = 0; col < num_cols; col++)
    {
        char *value = PQfname(result, col);
        unsigned long str_len = strlen(value);
        if (str_len > biggest_str)
        {
            biggest_str = str_len;
        }

        col_values[col] = value;
    }
    printf("\n");

    char ***row_values = malloc(sizeof(char **) * num_rows);
    for (int row = 0; row < num_rows; row++)
    {
        row_values[row] = malloc(sizeof(char *) * num_cols);
    }

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            char *value = PQgetvalue(result, row, col);
            unsigned long str_len = strlen(value);
            if (str_len > biggest_str)
            {
                biggest_str = str_len;
            }

            row_values[row][col] = value;
        }
    }

    draw_table(&(struct print_table_info){
        .amount_cols = num_cols,
        .amount_rows = num_rows,
        .cols = col_values,
        .rows = row_values});

    printf("Time: %fs\n", elapsed_time);

    free_results(row_values, num_rows);

    PQclear(result);
}

void terminate_connection(PGconn *conn) { PQfinish(conn); }
