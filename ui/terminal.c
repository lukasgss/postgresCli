#include <ctype.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define C_STRING "\001\033[0;32m\002"  // green
#define C_NUMBER "\001\033[0;33m\002"  // orange
#define C_FUNCTION "\033[0;33m"        // yellow
#define C_RESET "\001\033[0m\002"

char *highlight(const char *input)
{
    // TODO: use dynamic sized buffer
    static char buffer[4096];
    char *write_pos = buffer;
    const char *p = input;

    while (*p != '\0')
    {
        if (isspace(*p))
        {
            *write_pos++ = *p++;
            continue;
        }

        if (*p == '\'' || *p == '"')
        {
            char quote = *p;
            write_pos += sprintf(write_pos, "%s%c", C_STRING, *p++);

            while (*p && *p != quote)
            {
                *write_pos++ = *p++;
            }
            if (*p)
            {
                *write_pos++ = *p++;
            }

            write_pos += sprintf(write_pos, "%s", C_RESET);
            continue;
        }

        if (isalpha(*p))
        {
            size_t word_max_size = 32;
            char *word = malloc(word_max_size);
            if (word == NULL)
            {
                fprintf(stderr, "failed to allocate memory for word");
                exit(1);
            }

            bool contains_left_paren = false;
            bool contains_right_paren = false;
            size_t len = 0;

            while (*p && (isalnum(*p) || *p == '(' || *p == ')'))
            {
                if (len >= word_max_size - 1)
                {
                    word_max_size *= 2;
                    char *new_word = realloc(word, word_max_size);
                    if (new_word == NULL)
                    {
                        free(word);
                        fprintf(stderr, "failed to allocate memory for word");
                        return NULL;
                    }

                    word = new_word;
                }

                if (*p == '(')
                {
                    contains_left_paren = true;
                }
                else if (*p == ')')
                {
                    contains_right_paren = true;
                }
                word[len++] = *p++;
            }
            word[len] = '\0';

            if (contains_left_paren && contains_right_paren)
            {
                write_pos +=
                    sprintf(write_pos, "%s%s%s", C_FUNCTION, word, C_RESET);
            }
            else
            {
                write_pos += sprintf(write_pos, "%s", word);
            }

            free(word);

            continue;
        }

        if (isdigit(*p))
        {
            write_pos += sprintf(write_pos, "%s", C_NUMBER);

            while (*p && (isdigit(*p)))
            {
                *write_pos++ = *p++;
            }

            write_pos += sprintf(write_pos, "%s", C_RESET);
            continue;
        }

        *write_pos++ = *p++;
    }

    *write_pos = '\0';
    return buffer;
}

void custom_display(void)
{
    char *highlighted = highlight(rl_line_buffer);
    // clear line and redraw
    printf("\r\033[K%s%s", rl_prompt, highlighted);

    int cursor_pos = rl_point;
    int line_len = strlen(rl_line_buffer);
    if (cursor_pos < line_len)
    {
        printf("\033[%dD", line_len - cursor_pos);
    }

    fflush(stdout);
}

void init_readline(void) { rl_redisplay_function = custom_display; }

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
