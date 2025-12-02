#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

char *read_line(char *database_name)
{
    char prompt[256];
    snprintf(prompt, sizeof prompt, "%s> ", database_name);

    char *input = readline(prompt);

    if (input == NULL)
    {
        return NULL;
    }

    if (strcmp(input, "\\q") == 0)
    {
        free(input);
        return NULL;
    }

    if (*input)
    {
        add_history(input);
    }

    return input;
}
