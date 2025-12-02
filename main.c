#include <libpq-fe.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sql/postgres.h"
#include "ui/terminal.h"

int main(int argc, char **argv)
{
    PGconn *conn = connect_to_db("localhost", "rust_db", "postgres", "123456");
    char *input;

    init_readline();

    while ((input = readline("rust_db@localhost > ")) != NULL)
    {
        if (*input)
        {
            execute_statement(conn, input);
        }
        free(input);
    }

    terminate_connection(conn);

    printf("Goodbye!\n");
}
