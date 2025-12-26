#include <bits/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/cdefs.h>
#include <time.h>

#include "libpq-fe.h"
#include "postgres.h"
#include "table/table.h"

#define NANOSECONDS_PER_SEC 1000000000.0

static struct database_metadata db_metadata = {
    .amount_tables = 0, .tables = NULL};

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

    // TODO: make this use malloc instead of default buffer size
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

struct database_metadata const *get_db_metadata(void) { return &db_metadata; }

void fetch_db_metadata(PGconn *conn)
{
    PGresult *result = exec_to_db(
        conn,
        "SELECT table_name FROM information_schema.tables WHERE "
        "table_schema='public' AND table_type='BASE TABLE';");

    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "%s\n", PQerrorMessage(conn));
        PQclear(result);
        return;
    }

    int num_rows = PQntuples(result);

    char **tables = malloc(num_rows * sizeof(char *));
    if (tables == NULL)
    {
        fprintf(stderr, "failed to allocate memory to fetch all tables\n");
    }

    int num_fields = PQnfields(result);
    int subtract_from_position = 0;
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_fields; col++)
        {
            char *value = PQgetvalue(result, row, col);
            if (strcmp(value, "spatial_ref_sys") == 0)
            {
                subtract_from_position++;
                continue;
            }

            size_t table_name_len = strlen(value);

            tables[row - subtract_from_position] =
                malloc(table_name_len * sizeof(char) + 1);

            tables[row - subtract_from_position] = value;
        }
    }

    db_metadata = (struct database_metadata){
        .amount_tables = num_rows - subtract_from_position, .tables = tables};
}

void clear_db_metadata()
{
    for (size_t i = 0; i < db_metadata.amount_tables; i++)
    {
        free(db_metadata.tables[i]);
    }

    free(db_metadata.tables);
}

static bool is_text_table_name(const char *text)
{
    for (size_t table = 0; table < db_metadata.amount_tables; table++)
    {
        size_t table_name_len = strlen(db_metadata.tables[table]);

        for (size_t c = 0; text[c]; c++)
        {
            if (c > table_name_len - 1 ||
                text[c] != db_metadata.tables[table][c])
            {
                break;
            }

            return true;
        }
    }

    return false;
}

static char *add_quotes_table_names(const char *statement)
{
    size_t statement_len = strlen(statement);

    char *quoted_statement = malloc(statement_len * 2 + 1);
    if (!quoted_statement)
    {
        return NULL;
    }

    char *current_word = malloc(statement_len + 1);
    if (!current_word)
    {
        free(quoted_statement);
        return NULL;
    }

    size_t curr_word_index = 0;
    size_t curr_quoted_str_idx = 0;

    for (size_t i = 0; i <= statement_len; i++)
    {
        char c = statement[i];

        if (c == ' ' || c == '\0')
        {
            current_word[curr_word_index] = '\0';

            if (curr_word_index > 0)
            {
                if (is_text_table_name(current_word))
                {
                    quoted_statement[curr_quoted_str_idx++] = '"';
                    for (size_t j = 0; j < curr_word_index; j++)
                    {
                        quoted_statement[curr_quoted_str_idx++] =
                            current_word[j];
                    }
                    quoted_statement[curr_quoted_str_idx++] = '"';
                }
                else
                {
                    for (size_t j = 0; j < curr_word_index; j++)
                    {
                        quoted_statement[curr_quoted_str_idx++] =
                            current_word[j];
                    }
                }
            }

            if (c == ' ')
            {
                quoted_statement[curr_quoted_str_idx++] = ' ';
            }

            curr_word_index = 0;
        }
        else
        {
            current_word[curr_word_index++] = c;
        }
    }

    quoted_statement[curr_quoted_str_idx] = '\0';

    free(current_word);
    return quoted_statement;
}

static bool is_exit_statement(const char *statement)
{
    return strcasecmp(statement, "exit") == 0;
}

void execute_statement(PGconn *conn, char *statement)
{
    if (is_exit_statement(statement))
    {
        printf("\n");
        exit(0);
    }

    // can't create a value if not a pointer, since type
    // isn't defined in the header file
    PGresult *result = NULL;

    char *quoted_statement = add_quotes_table_names(statement);

    double elapsed_time = measure_statement_exec_time_in_secs(
        exec_to_db, conn, quoted_statement, &result);

    free(quoted_statement);

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
