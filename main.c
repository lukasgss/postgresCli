#include <libpq-fe.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flags/flags.h"
#include "sql/postgres.h"
#include "ui/terminal.h"

int main(int argc, char **argv)
{
    struct conn_info conn_info = parse_flags(argc, argv);

    PGconn *conn = connect_to_db(
        conn_info.host, conn_info.dbname, conn_info.user, conn_info.password);
    char *input;

    init_readline();

    char *readline_str = get_readline_prompt(conn_info.dbname, conn_info.host);

    while ((input = readline(readline_str)) != NULL)
    {
        if (*input)
        {
            if (strcmp(input, "\\q") == 0)
            {
                printf("\n");
                break;
            }

            execute_statement(conn, input);
        }
        free(input);
    }

    free(readline_str);

    terminate_connection(conn);

    printf("Goodbye!\n");
}
